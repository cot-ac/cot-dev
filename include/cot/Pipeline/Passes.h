//===- Passes.h - Pass forward declarations -------------------*- C++ -*-===//
#ifndef COT_PIPELINE_PASSES_H
#define COT_PIPELINE_PASSES_H

#include <memory>

namespace mlir {
class Pass;
} // namespace mlir

namespace cot {

/// CIR->LLVM conversion pass. Applies all construct lowering patterns.
std::unique_ptr<mlir::Pass> createCIRToLLVMPass();

} // namespace cot

#endif // COT_PIPELINE_PASSES_H
