//===- SemaStep.h - CIRSema step interface --------------------*- C++ -*-===//
//
// The interface that constructs implement to participate in the CIRSema
// single-walk semantic analysis pass.
//
// Steps run in fixed position order for each op:
//   Comptime → Generics → Types → Ownership
//
// Reference: Zig Sema (single-walk model), Swift SILGen + mandatory passes.
//
//===----------------------------------------------------------------------===//
#ifndef COT_PIPELINE_SEMASTEP_H
#define COT_PIPELINE_SEMASTEP_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "llvm/ADT/StringRef.h"

namespace cot {

class CIRSema; // forward declaration

/// A step in the CIRSema single-walk pass.
///
/// Each construct registers one or more steps. The framework calls visitOp()
/// for every op in the module, in position order. Steps at the same position
/// run in registration order (construct dependency order).
///
/// For transforms that need whole-function iterative analysis (dataflow,
/// fixed-point iteration), use Construct::addTransformers() instead.
class SemaStep {
public:
  /// Fixed positions in the CIRSema walk. The ordering is an invariant.
  enum Position {
    Comptime  = 0, ///< Evaluate comptime-known ops, fold constants
    Generics  = 1, ///< Resolve generic_apply, specialize functions
    Types     = 2, ///< Type check, insert casts
    Ownership = 3, ///< Insert copy_value/destroy_value for ARC types
  };

  virtual ~SemaStep() = default;

  /// Human-readable name for logging/debugging.
  virtual llvm::StringRef getName() const = 0;

  /// Which position in the walk order this step occupies.
  virtual Position getPosition() const = 0;

  /// Called for each op during the CIRSema walk.
  /// Return true if this step handled the op (may have modified/replaced it).
  /// Return false to pass the op to the next step.
  virtual bool visitOp(mlir::Operation *op, CIRSema &sema) { return false; }

  /// Called once after the walk completes.
  /// Use for: generating new functions (test runners, witness thunks),
  /// module-level cleanup, or any work that creates new ops.
  virtual void finalize(mlir::ModuleOp module, CIRSema &sema) {}
};

} // namespace cot

#endif // COT_PIPELINE_SEMASTEP_H
