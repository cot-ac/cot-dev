//===- Pipeline.cpp - PipelineBuilder implementation ----------*- C++ -*-===//
//
// Reference: Flang splits Transforms/ from CodeGen/.
// Pass ordering reference: Flang lib/Optimizer/Transforms/ +
//   lib/Optimizer/CodeGen/
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Pipeline.h"
#include "cot/Pipeline/Passes.h"
#include "cot/Pipeline/Diagnostics.h"
#include "cot/Construct/Construct.h"
#include "cot/CIR/CIRDialect.h"

#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"

using namespace mlir;
using namespace cot;

PipelineBuilder::PipelineBuilder(MLIRContext *ctx) : ctx(ctx) {}

void PipelineBuilder::enableDebugPipeline(llvm::raw_ostream &os) {
  debugOs = &os;
}

void PipelineBuilder::addPreSemaPass(std::unique_ptr<Pass> pass) {
  preSemaPasses.push_back(std::move(pass));
}

void PipelineBuilder::addPostSemaPass(std::unique_ptr<Pass> pass) {
  postSemaPasses.push_back(std::move(pass));
}

void PipelineBuilder::addPostLoweringPass(std::unique_ptr<Pass> pass) {
  postLoweringPasses.push_back(std::move(pass));
}

void PipelineBuilder::addExtraLinkArg(llvm::StringRef arg) {
  extraLinkArgs.push_back(arg.str());
}

//===----------------------------------------------------------------------===//
// Sema stages: the fixed ordering that makes everything work
//===----------------------------------------------------------------------===//

LogicalResult PipelineBuilder::runSemaStages(ModuleOp module) {
  PassManager pm(ctx);
  pm.enableVerifier(true);

  // Pipeline debugger: dump IR before/after each pass
  if (debugOs)
    pm.addInstrumentation(createPipelineDebugInstrumentation(*debugOs));

  // Construct-provided passes run in two phases:
  // Phase 1 (pre-sema): GenericSpecializer, WitnessThunkGenerator, etc.
  // Phase 2 (post-sema): SemanticAnalysis, TestRunnerGenerator, etc.
  //
  // We collect them into separate PMs and run sequentially because
  // some constructs add nested passes (per-function) while others
  // add module passes, and they must be in the right PM context.

  // Phase 1: Pre-sema construct passes
  {
    PassManager preSemaPM(ctx);
    PassManager postSemaPM(ctx); // collected but run later
    for (auto &construct : cot::getConstructRegistry())
      construct->addTransformers(preSemaPM, postSemaPM);

    // Run pre-sema passes (GenericSpecializer, etc.)
    if (preSemaPM.size() > 0)
      if (failed(preSemaPM.run(module)))
        return failure();

    // External pre-sema passes
    for (auto &pass : preSemaPasses)
      pm.addPass(std::move(pass));

    // Phase 2: Post-sema construct passes (Sema, test runner, etc.)
    if (postSemaPM.size() > 0) {
      // Run external pre-sema first
      if (pm.size() > 0)
        if (failed(pm.run(module)))
          return failure();
      // Then run post-sema construct passes
      if (failed(postSemaPM.run(module)))
        return failure();
    }
  }

  // External post-sema passes
  {
    PassManager postPM(ctx);
    for (auto &pass : postSemaPasses)
      postPM.addPass(std::move(pass));
    if (postPM.size() > 0)
      if (failed(postPM.run(module)))
        return failure();
  }

  return success();
}

//===----------------------------------------------------------------------===//
// Lowering stages: CIR -> LLVM dialect
//===----------------------------------------------------------------------===//

LogicalResult PipelineBuilder::runLoweringStages(ModuleOp module) {
  PassManager pm(ctx);

  // Pipeline debugger: dump IR before/after each pass
  if (debugOs)
    pm.addInstrumentation(createPipelineDebugInstrumentation(*debugOs));

  // CIR->LLVM conversion
  pm.addPass(createCIRToLLVMPass());

  // Clean up materializations (errata E2/E14)
  pm.addPass(mlir::createReconcileUnrealizedCastsPass());

  // External post-lowering passes
  for (auto &pass : postLoweringPasses)
    pm.addPass(std::move(pass));

  return pm.run(module);
}

//===----------------------------------------------------------------------===//
// Codegen: LLVM dialect -> native binary
//===----------------------------------------------------------------------===//

LogicalResult PipelineBuilder::runCodegen(ModuleOp module,
                                          StringRef outputPath) {
  // Initialize LLVM targets
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  // Register dialect translations
  registerBuiltinDialectTranslation(*ctx);
  registerLLVMDialectTranslation(*ctx);

  // Translate MLIR LLVM dialect -> LLVM IR
  llvm::LLVMContext llvmCtx;
  auto llvmModule = translateModuleToLLVMIR(module, llvmCtx);
  if (!llvmModule)
    return failure();

  // Set target triple
  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  llvmModule->setTargetTriple(targetTriple);

  // Write LLVM IR to temp file, then compile with clang
  std::string llPath = (outputPath + ".ll").str();
  {
    std::error_code ec;
    llvm::raw_fd_ostream dest(llPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
      module.emitError("failed to open output: " + ec.message());
      return failure();
    }
    llvmModule->print(dest, nullptr);
    dest.flush();
  }

  // Find clang in the same directory as the LLVM we linked against,
  // or fall back to system cc
  auto clangPath = llvm::sys::findProgramByName("clang");
  if (!clangPath)
    clangPath = llvm::sys::findProgramByName("cc");
  if (!clangPath) {
    module.emitError("neither clang nor cc found in PATH");
    return failure();
  }

  // Compile: clang -o output output.ll [extra args]
  llvm::SmallVector<llvm::StringRef> linkArgs = {
      *clangPath, "-o", outputPath, llPath,
      "-Wno-override-module"};
  for (const auto &arg : extraLinkArgs)
    linkArgs.push_back(arg);
  int linkResult = llvm::sys::ExecuteAndWait(*clangPath, linkArgs);
  if (linkResult != 0)
    return failure();

  // Clean up temp file
  llvm::sys::fs::remove(llPath);
  return success();
}

//===----------------------------------------------------------------------===//
// Public entry points
//===----------------------------------------------------------------------===//

LogicalResult PipelineBuilder::emitBinary(ModuleOp module,
                                          StringRef outputPath) {
  if (failed(runSemaStages(module)))
    return failure();
  if (failed(runLoweringStages(module)))
    return failure();
  return runCodegen(module, outputPath);
}

LogicalResult PipelineBuilder::runToTypedCIR(ModuleOp module) {
  return runSemaStages(module);
}

LogicalResult PipelineBuilder::runToLLVM(ModuleOp module) {
  if (failed(runSemaStages(module)))
    return failure();
  return runLoweringStages(module);
}
