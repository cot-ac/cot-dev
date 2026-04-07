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

//===----------------------------------------------------------------------===//
// Sema stages: the fixed ordering that makes everything work
//===----------------------------------------------------------------------===//

LogicalResult PipelineBuilder::runSemaStages(ModuleOp module) {
  PassManager pm(ctx);
  pm.enableVerifier(true);

  // Pipeline debugger: dump IR before/after each pass
  if (debugOs)
    pm.addInstrumentation(createPipelineDebugInstrumentation(*debugOs));

  // Phase 1-2: passes not yet implemented are simply not added.
  // WitnessThunkGenerator, GenericSpecializer, OwnershipEliminator,
  // ARCOptimizer are added as their construct repos are built.

  // External pre-sema passes
  for (auto &pass : preSemaPasses)
    pm.addPass(std::move(pass));

  // SemanticAnalysis will be added by cot-core construct registration.

  // External post-sema passes
  for (auto &pass : postSemaPasses)
    pm.addPass(std::move(pass));

  // Skip running an empty pipeline
  if (pm.size() == 0)
    return success();

  return pm.run(module);
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

  // Compile: clang -o output output.ll
  llvm::SmallVector<llvm::StringRef> linkArgs = {
      *clangPath, "-o", outputPath, llPath,
      "-Wno-override-module"};
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
