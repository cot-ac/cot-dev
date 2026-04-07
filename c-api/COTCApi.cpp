//===- COTCApi.cpp - COT Pipeline C API implementation --------*- C++ -*-===//
#include "cot-c/COTCApi.h"
#include "cot/Pipeline/Pipeline.h"
#include "cot/CIR/CIRDialect.h"

#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Pass.h"
#include "mlir/CAPI/Support.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

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
