//===- Diagnostics.h - Pipeline debugger + diagnostics ---------*- C++ -*-===//
//
// PassInstrumentation that dumps IR before/after each pass stage.
// Enabled via PipelineBuilder::enableDebugPipeline().
//
//===----------------------------------------------------------------------===//
#ifndef COT_PIPELINE_DIAGNOSTICS_H
#define COT_PIPELINE_DIAGNOSTICS_H

#include "mlir/Pass/PassInstrumentation.h"
#include "llvm/Support/raw_ostream.h"

namespace cot {

/// Pipeline debugger: dumps IR before and after each pass.
/// When a pass fails, dumps both the before-snapshot and the failed IR,
/// making it immediately clear what went wrong.
class PipelineDebugInstrumentation : public mlir::PassInstrumentation {
public:
  explicit PipelineDebugInstrumentation(llvm::raw_ostream &os);
  ~PipelineDebugInstrumentation() override;

  void runBeforePass(mlir::Pass *pass, mlir::Operation *op) override;
  void runAfterPass(mlir::Pass *pass, mlir::Operation *op) override;
  void runAfterPassFailed(mlir::Pass *pass, mlir::Operation *op) override;

private:
  llvm::raw_ostream &os;
  std::string beforeSnapshot;
};

/// Create the pipeline debug instrumentation.
std::unique_ptr<mlir::PassInstrumentation>
createPipelineDebugInstrumentation(llvm::raw_ostream &os);

} // namespace cot

#endif // COT_PIPELINE_DIAGNOSTICS_H
