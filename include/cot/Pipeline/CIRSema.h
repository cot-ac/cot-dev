//===- CIRSema.h - Single-walk semantic analysis pass ---------*- C++ -*-===//
//
// CIRSema walks every op in the module once, dispatching to construct-
// registered steps in fixed order: Comptime → Generics → Types → Ownership.
//
// This replaces the separate GenericSpecializer + SemanticAnalysis +
// OwnershipEliminator passes with one coordinated walk that shares state.
//
// Reference: Zig Sema (34,642-line single-walk), Swift SILGen.
//
//===----------------------------------------------------------------------===//
#ifndef COT_PIPELINE_CIRSEMA_H
#define COT_PIPELINE_CIRSEMA_H

#include "cot/Pipeline/SemaStep.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/IR/Attributes.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <memory>

namespace mlir {
class Pass;
} // namespace mlir

namespace cot {

/// The CIRSema state — carried during the single walk.
///
/// This is the equivalent of Zig's Sema struct: it holds all mutable
/// state that steps read and write during analysis.
class CIRSema {
public:
  /// The MLIR context.
  mlir::MLIRContext *ctx = nullptr;

  /// The module being analyzed.
  mlir::ModuleOp module;

  /// Module-level symbol table (shared across all steps).
  std::unique_ptr<mlir::SymbolTable> symbolTable;

  // ===------------------------------------------------------------------===
  // Comptime state (from Zig Sema)
  // ===------------------------------------------------------------------===

  /// Values known at compile time: op result → constant attribute.
  /// Populated by the comptime step; read by generics and types steps.
  llvm::DenseMap<mlir::Value, mlir::Attribute> comptimeValues;

  /// Memoized comptime function calls: mangled key → result attribute.
  /// Key is "func_name:arg0:arg1:..." for deduplication.
  /// Prevents re-evaluation of the same comptime call.
  llvm::StringMap<mlir::Attribute> memoizedCalls;

  /// Comptime evaluation limits (from Zig's branch_quota).
  uint32_t branchQuota = 1000;
  uint32_t branchCount = 0;

  // ===------------------------------------------------------------------===
  // Step registry
  // ===------------------------------------------------------------------===

  /// Register a step. Steps are sorted by position before the walk.
  void addStep(std::unique_ptr<SemaStep> step);

  /// Run the single walk over the module.
  /// Called by the CIRSema MLIR pass.
  mlir::LogicalResult run(mlir::ModuleOp module);

private:
  /// Steps grouped by position. Index = SemaStep::Position.
  llvm::SmallVector<std::unique_ptr<SemaStep>> steps_[4];

  /// Walk all ops in a region, dispatching to steps.
  void walkRegion(mlir::Region &region);

  /// Walk all ops in a block, dispatching to steps.
  void walkBlock(mlir::Block &block);

  /// Dispatch a single op to all steps in order.
  void dispatchOp(mlir::Operation *op);
};

/// Create the CIRSema MLIR pass.
/// Collects steps from all registered constructs, then runs the single walk.
std::unique_ptr<mlir::Pass> createCIRSemaPass();

} // namespace cot

#endif // COT_PIPELINE_CIRSEMA_H
