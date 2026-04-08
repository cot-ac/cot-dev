//===- COTCApi.cpp - COT Pipeline C API implementation --------*- C++ -*-===//
#include "cot-c/COTCApi.h"
#include "cot/Pipeline/Pipeline.h"
#include "cot/CIR/CIRDialect.h"

#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Pass.h"
#include "mlir/CAPI/Support.h"
#include "mlir-c/Support.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

#include <cstdlib>
#include <cstring>

void cotInitContext(MlirContext ctx) {
  auto *context = unwrap(ctx);
  context->getOrLoadDialect<cir::CIRDialect>();
  context->getOrLoadDialect<mlir::func::FuncDialect>();
  context->getOrLoadDialect<mlir::LLVM::LLVMDialect>();
}

MlirLogicalResult cotRunSema(MlirModule module) {
  auto *ctx = unwrap(module)->getContext();
  cot::PipelineBuilder pipeline(ctx);
  return wrap(pipeline.runToTypedCIR(
      mlir::cast<mlir::ModuleOp>(unwrap(module))));
}

MlirLogicalResult cotLowerToLLVM(MlirModule module) {
  auto *ctx = unwrap(module)->getContext();
  cot::PipelineBuilder pipeline(ctx);
  return wrap(pipeline.runToLLVM(
      mlir::cast<mlir::ModuleOp>(unwrap(module))));
}

MlirLogicalResult cotEmitBinary(MlirModule module,
                                const char *outputPath) {
  auto *ctx = unwrap(module)->getContext();
  cot::PipelineBuilder pipeline(ctx);
  return wrap(pipeline.emitBinary(
      mlir::cast<mlir::ModuleOp>(unwrap(module)), outputPath));
}

// Opaque handle wrapping
struct CotPipelineBuilderS {
  cot::PipelineBuilder impl;
  CotPipelineBuilderS(mlir::MLIRContext *ctx) : impl(ctx) {}
};

CotPipelineBuilder cotPipelineBuilderCreate(MlirContext ctx) {
  return new CotPipelineBuilderS(unwrap(ctx));
}

void cotPipelineBuilderDestroy(CotPipelineBuilder builder) {
  delete builder;
}

void cotPipelineBuilderAddPreSemaPass(CotPipelineBuilder builder,
                                      MlirPass pass) {
  builder->impl.addPreSemaPass(
      std::unique_ptr<mlir::Pass>(unwrap(pass)));
}

void cotPipelineBuilderAddPostSemaPass(CotPipelineBuilder builder,
                                       MlirPass pass) {
  builder->impl.addPostSemaPass(
      std::unique_ptr<mlir::Pass>(unwrap(pass)));
}

void cotPipelineBuilderAddPostLoweringPass(CotPipelineBuilder builder,
                                           MlirPass pass) {
  builder->impl.addPostLoweringPass(
      std::unique_ptr<mlir::Pass>(unwrap(pass)));
}

MlirLogicalResult cotPipelineBuilderRunToTypedCIR(
    CotPipelineBuilder builder, MlirModule module) {
  return wrap(builder->impl.runToTypedCIR(
      mlir::cast<mlir::ModuleOp>(unwrap(module))));
}

MlirLogicalResult cotPipelineBuilderRunToLLVM(
    CotPipelineBuilder builder, MlirModule module) {
  return wrap(builder->impl.runToLLVM(
      mlir::cast<mlir::ModuleOp>(unwrap(module))));
}

MlirLogicalResult cotPipelineBuilderEmitBinary(
    CotPipelineBuilder builder, MlirModule module,
    const char *outputPath) {
  return wrap(builder->impl.emitBinary(
      mlir::cast<mlir::ModuleOp>(unwrap(module)), outputPath));
}

//===----------------------------------------------------------------------===//
// Transform authoring — external pass convenience wrappers
//===----------------------------------------------------------------------===//

namespace {

// Bridge struct: stores the user's simple callback and adapts it to the
// full MlirExternalPassCallbacks signature that MLIR requires.
struct CotPassBridge {
  CotPassRunFn userRun;
  void *userData;
  bool isFuncPass; // If true, iterate over func.func ops in the module.
};

// Global type ID allocator (leaked on exit — fine for a compiler).
static MlirTypeIDAllocator getTypeIDAllocator() {
  static MlirTypeIDAllocator allocator = mlirTypeIDAllocatorCreate();
  return allocator;
}

// MLIR callback trampolines
static void cotPassConstruct(void *) {}

static void cotPassDestruct(void *ud) {
  free(ud);
}

static MlirLogicalResult cotPassInitialize(MlirContext, void *) {
  return mlirLogicalResultSuccess();
}

static void *cotPassClone(void *ud) {
  auto *orig = static_cast<CotPassBridge *>(ud);
  auto *copy = static_cast<CotPassBridge *>(malloc(sizeof(CotPassBridge)));
  *copy = *orig;
  return copy;
}

static void cotPassRun(MlirOperation op, MlirExternalPass,
                       void *ud) {
  auto *bridge = static_cast<CotPassBridge *>(ud);
  if (!bridge->isFuncPass) {
    // Module pass: call user callback directly with the module op.
    bridge->userRun(op, bridge->userData);
    return;
  }
  // Func pass: iterate over func.func ops in the module and call
  // the user callback for each one.
  intptr_t nRegions = mlirOperationGetNumRegions(op);
  for (intptr_t r = 0; r < nRegions; r++) {
    MlirRegion region = mlirOperationGetRegion(op, r);
    MlirBlock block = mlirRegionGetFirstBlock(region);
    while (!mlirBlockIsNull(block)) {
      MlirOperation child = mlirBlockGetFirstOperation(block);
      while (!mlirOperationIsNull(child)) {
        // Save next before the callback might modify the block.
        MlirOperation next = mlirOperationGetNextInBlock(child);
        MlirIdentifier name = mlirOperationGetName(child);
        MlirStringRef nameStr = mlirIdentifierStr(name);
        if (nameStr.length == 9 &&
            memcmp(nameStr.data, "func.func", 9) == 0) {
          bridge->userRun(child, bridge->userData);
        }
        child = next;
      }
      block = mlirBlockGetNextInRegion(block);
    }
  }
}

} // namespace

MlirPass cotCreateModulePass(const char *name,
                              const char *description,
                              CotPassRunFn run,
                              void *userData) {
  auto *bridge = static_cast<CotPassBridge *>(
      malloc(sizeof(CotPassBridge)));
  bridge->userRun = run;
  bridge->userData = userData;
  bridge->isFuncPass = false;

  MlirTypeID passID =
      mlirTypeIDAllocatorAllocateTypeID(getTypeIDAllocator());

  MlirExternalPassCallbacks callbacks;
  callbacks.construct = cotPassConstruct;
  callbacks.destruct = cotPassDestruct;
  callbacks.initialize = cotPassInitialize;
  callbacks.clone = cotPassClone;
  callbacks.run = cotPassRun;

  // Empty opName = generic pass that can run on any op (including module).
  MlirStringRef emptyOpName = mlirStringRefCreateFromCString("");
  return mlirCreateExternalPass(
      passID,
      mlirStringRefCreateFromCString(name),
      mlirStringRefCreateFromCString(name),
      mlirStringRefCreateFromCString(description),
      emptyOpName,
      /*nDependentDialects=*/0, /*dependentDialects=*/nullptr,
      callbacks, bridge);
}

MlirPass cotCreateFuncPass(const char *name,
                            const char *description,
                            CotPassRunFn run,
                            void *userData) {
  auto *bridge = static_cast<CotPassBridge *>(
      malloc(sizeof(CotPassBridge)));
  bridge->userRun = run;
  bridge->userData = userData;
  bridge->isFuncPass = true;

  MlirTypeID passID =
      mlirTypeIDAllocatorAllocateTypeID(getTypeIDAllocator());

  MlirExternalPassCallbacks callbacks;
  callbacks.construct = cotPassConstruct;
  callbacks.destruct = cotPassDestruct;
  callbacks.initialize = cotPassInitialize;
  callbacks.clone = cotPassClone;
  callbacks.run = cotPassRun;

  // Empty opName = runs at module level; trampoline iterates func ops.
  MlirStringRef emptyOpName = mlirStringRefCreateFromCString("");
  return mlirCreateExternalPass(
      passID,
      mlirStringRefCreateFromCString(name),
      mlirStringRefCreateFromCString(name),
      mlirStringRefCreateFromCString(description),
      emptyOpName,
      /*nDependentDialects=*/0, /*dependentDialects=*/nullptr,
      callbacks, bridge);
}
