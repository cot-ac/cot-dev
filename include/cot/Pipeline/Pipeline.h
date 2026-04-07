//===- Pipeline.h - PipelineBuilder declaration ---------------*- C++ -*-===//
#ifndef COT_PIPELINE_PIPELINE_H
#define COT_PIPELINE_PIPELINE_H

#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"

#include <functional>
#include <string>
#include <vector>

namespace cot {

class PipelineBuilder {
public:
  explicit PipelineBuilder(mlir::MLIRContext *ctx);

  //--- Debug ---

  /// Enable pipeline debugging: dumps IR before/after each pass.
  /// Must be called before emitBinary/runToTypedCIR/runToLLVM.
  void enableDebugPipeline(llvm::raw_ostream &os);

  //--- Extension points for external passes ---

  /// Add a pass that runs BEFORE type checking.
  /// Use for: witness thunk generation, generic preparation.
  void addPreSemaPass(std::unique_ptr<mlir::Pass> pass);

  /// Add a pass that runs AFTER type checking.
  /// Primary extension point for language-specific transforms.
  /// Use for: comptime evaluation, custom optimizations.
  void addPostSemaPass(std::unique_ptr<mlir::Pass> pass);

  /// Add a pass that runs AFTER CIR->LLVM lowering.
  /// Use for: LLVM-level optimizations, target-specific transforms.
  void addPostLoweringPass(std::unique_ptr<mlir::Pass> pass);

  //--- Pipeline execution ---

  /// Run the full pipeline: CIR -> typed CIR -> LLVM -> native binary.
  /// outputPath: path for the output binary (e.g., "a.out").
  mlir::LogicalResult emitBinary(mlir::ModuleOp module,
                                 llvm::StringRef outputPath);

  /// Run up to typed CIR (after Sema). For emit-cir.
  mlir::LogicalResult runToTypedCIR(mlir::ModuleOp module);

  /// Run up to LLVM dialect (after lowering). For emit-llvm.
  mlir::LogicalResult runToLLVM(mlir::ModuleOp module);

private:
  /// Run semantic analysis stages: witness thunks -> generic specializer ->
  /// sema -> verify -> ownership eliminator -> ARC optimizer.
  mlir::LogicalResult runSemaStages(mlir::ModuleOp module);

  /// Run lowering stages: witness table generation -> CIR->LLVM.
  mlir::LogicalResult runLoweringStages(mlir::ModuleOp module);

  /// Run codegen: LLVM dialect -> LLVM IR -> object file -> link.
  mlir::LogicalResult runCodegen(mlir::ModuleOp module,
                                 llvm::StringRef outputPath);

  mlir::MLIRContext *ctx;
  llvm::raw_ostream *debugOs = nullptr;
  std::vector<std::unique_ptr<mlir::Pass>> preSemaPasses;
  std::vector<std::unique_ptr<mlir::Pass>> postSemaPasses;
  std::vector<std::unique_ptr<mlir::Pass>> postLoweringPasses;
};

} // namespace cot

#endif // COT_PIPELINE_PIPELINE_H
