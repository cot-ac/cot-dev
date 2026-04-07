//===- e2e_phase2_test.cpp - Phase 2 gate test ----------------*- C++ -*-===//
//
// Programmatically build:
//   func @main() -> i32 {
//     %c = cir.constant 42 : i32
//     %one = cir.constant 1 : i32
//     %r = cir.add %c, %one : i32
//     return %r : i32
//   }
//
// Run PipelineBuilder::emitBinary(). Execute output. Exit code = 43.
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Pipeline.h"
#include "cot/Construct/Construct.h"
#include "cot/CIR/CIRDialect.h"
#include "cot-core/Ops.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"

#include "llvm/Support/raw_ostream.h"

using namespace mlir;

int main() {
  // Set up MLIR context with required dialects
  MLIRContext ctx;
  ctx.getOrLoadDialect<cir::CIRDialect>();
  ctx.getOrLoadDialect<func::FuncDialect>();
  ctx.getOrLoadDialect<LLVM::LLVMDialect>();
  ctx.getOrLoadDialect<arith::ArithDialect>();

  // Register construct ops (cot-core linked via -force_load)
  for (auto &construct : cot::getConstructRegistry())
    construct->registerOpsAndTypes(ctx);

  // Build the module
  OpBuilder builder(&ctx);
  auto loc = builder.getUnknownLoc();
  auto module = ModuleOp::create(loc);

  // func @main() -> i32
  auto i32Type = builder.getI32Type();
  auto funcType = builder.getFunctionType({}, {i32Type});
  builder.setInsertionPointToEnd(module.getBody());
  auto mainFunc = builder.create<func::FuncOp>(loc, "main", funcType);

  // Entry block
  auto *entryBlock = mainFunc.addEntryBlock();
  builder.setInsertionPointToStart(entryBlock);

  // %c = cir.constant 42 : i32
  auto constant42 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(42));

  // %one = cir.constant 1 : i32
  auto constant1 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(1));

  // %r = cir.add %c, %one : i32
  auto addResult = builder.create<cir::AddOp>(
      loc, i32Type, constant42, constant1);

  // return %r : i32
  builder.create<func::ReturnOp>(loc, ValueRange{addResult});

  // Run the pipeline
  cot::PipelineBuilder pipeline(&ctx);
  auto result = pipeline.emitBinary(module, "test_output_p2");
  if (failed(result)) {
    llvm::errs() << "Phase 2 pipeline failed!\n";
    return 1;
  }

  llvm::outs() << "Phase 2 binary emitted. Run ./test_output_p2\n";
  return 0;
}
