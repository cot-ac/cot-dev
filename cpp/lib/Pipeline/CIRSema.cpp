//===- CIRSema.cpp - Single-walk semantic analysis pass --------*- C++ -*-===//
//
// Walks every op in the module once, dispatching to construct-registered
// steps in fixed order: Comptime → Generics → Types → Ownership.
//
// Reference: Zig src/Sema.zig (analyzeBodyInner dispatch loop),
//            Swift lib/SILGen + mandatory diagnostic passes.
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/CIRSema.h"
#include "cot/Construct/Construct.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

using namespace mlir;
using namespace cot;

//===----------------------------------------------------------------------===//
// CIRSema — step registry + walk
//===----------------------------------------------------------------------===//

void CIRSema::addStep(std::unique_ptr<SemaStep> step) {
  auto pos = static_cast<unsigned>(step->getPosition());
  steps_[pos].push_back(std::move(step));
}

LogicalResult CIRSema::run(ModuleOp mod) {
  module = mod;
  ctx = mod.getContext();
  symbolTable = std::make_unique<SymbolTable>(mod);

  // Walk all ops in the module
  walkRegion(mod.getBodyRegion());

  // Finalize: let steps generate new functions, clean up, etc.
  for (unsigned pos = 0; pos < 4; pos++) {
    for (auto &step : steps_[pos]) {
      step->finalize(module, *this);
    }
  }

  return success();
}

void CIRSema::walkRegion(Region &region) {
  for (auto &block : region)
    walkBlock(block);
}

void CIRSema::walkBlock(Block &block) {
  // Collect ops first — steps may modify/erase ops during dispatch.
  // Walking a list while modifying it is undefined behavior.
  SmallVector<Operation *> ops;
  for (auto &op : block)
    ops.push_back(&op);

  for (auto *op : ops) {
    // Skip ops that were erased by a previous step
    if (op->getBlock() == nullptr)
      continue;

    // Dispatch this op to all steps
    dispatchOp(op);

    // Walk nested regions (function bodies, block regions, etc.)
    for (auto &region : op->getRegions())
      walkRegion(region);
  }
}

void CIRSema::dispatchOp(Operation *op) {
  // Call steps in position order: Comptime → Generics → Types → Ownership
  for (unsigned pos = 0; pos < 4; pos++) {
    for (auto &step : steps_[pos]) {
      if (step->visitOp(op, *this)) {
        // Step handled this op — it may have been replaced or erased.
        // Check if op was erased before continuing.
        if (op->getBlock() == nullptr)
          return;
        // Op was modified but not erased. Continue to next position.
        break;
      }
    }
  }
}

//===----------------------------------------------------------------------===//
// CIRSema MLIR pass wrapper
//===----------------------------------------------------------------------===//

namespace {

struct CIRSemaPass
    : public PassWrapper<CIRSemaPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CIRSemaPass)

  StringRef getArgument() const override { return "cir-sema"; }
  StringRef getDescription() const override {
    return "CIR semantic analysis: comptime, generics, types, ownership";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<func::FuncDialect>();
  }

  void runOnOperation() override {
    CIRSema sema;

    // Collect steps from all registered constructs
    for (auto &construct : cot::getConstructRegistry())
      construct->registerSemaSteps(sema);

    // Run the single walk
    if (failed(sema.run(getOperation())))
      return signalPassFailure();
  }
};

} // anonymous namespace

std::unique_ptr<Pass> cot::createCIRSemaPass() {
  return std::make_unique<CIRSemaPass>();
}
