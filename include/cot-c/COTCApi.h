//===- COTCApi.h - COT Pipeline C API -------------------------*- C -*-===//
//
// C API for driving the COT compilation pipeline.
// Bindings wrap these functions — they are the stable ABI.
//
//===----------------------------------------------------------------------===//
#ifndef COT_C_API_COTCAPI_H
#define COT_C_API_COTCAPI_H

#include "mlir-c/IR.h"
#include "mlir-c/Pass.h"

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Context initialization
//===----------------------------------------------------------------------===//

/// Initialize the MLIR context with CIR, Func, and LLVM dialects.
/// Call once before creating any modules.
void cotInitContext(MlirContext ctx);

//===----------------------------------------------------------------------===//
// Simple pipeline (convenience functions)
//===----------------------------------------------------------------------===//

/// Run semantic analysis on the module (type checking + cast insertion).
MlirLogicalResult cotRunSema(MlirModule module);

/// Lower CIR to LLVM dialect.
MlirLogicalResult cotLowerToLLVM(MlirModule module);

/// Full pipeline: CIR -> sema -> lower -> codegen -> native binary.
MlirLogicalResult cotEmitBinary(MlirModule module,
                                const char *outputPath);

//===----------------------------------------------------------------------===//
// Configurable pipeline builder
//===----------------------------------------------------------------------===//

typedef struct CotPipelineBuilderS *CotPipelineBuilder;

/// Create a new pipeline builder for the given context.
CotPipelineBuilder cotPipelineBuilderCreate(MlirContext ctx);

/// Destroy a pipeline builder.
void cotPipelineBuilderDestroy(CotPipelineBuilder builder);

/// Add a pass before semantic analysis.
void cotPipelineBuilderAddPreSemaPass(CotPipelineBuilder builder,
                                      MlirPass pass);

/// Add a pass after semantic analysis (primary extension point).
void cotPipelineBuilderAddPostSemaPass(CotPipelineBuilder builder,
                                       MlirPass pass);

/// Add a pass after CIR->LLVM lowering.
void cotPipelineBuilderAddPostLoweringPass(CotPipelineBuilder builder,
                                           MlirPass pass);

/// Run pipeline up to typed CIR (for emit-cir).
MlirLogicalResult cotPipelineBuilderRunToTypedCIR(
    CotPipelineBuilder builder, MlirModule module);

/// Run pipeline up to LLVM dialect (for emit-llvm).
MlirLogicalResult cotPipelineBuilderRunToLLVM(
    CotPipelineBuilder builder, MlirModule module);

/// Run full pipeline to native binary.
MlirLogicalResult cotPipelineBuilderEmitBinary(
    CotPipelineBuilder builder, MlirModule module,
    const char *outputPath);

#ifdef __cplusplus
}
#endif

#endif // COT_C_API_COTCAPI_H
