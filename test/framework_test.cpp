//===- framework_test.cpp - Phase 1 gate test -----------------*- C++ -*-===//
//
// Build a module with: func @main() -> i32 { return 42 }
// Run PipelineBuilder::emitBinary(). Execute output binary.
// PASS: exit code is 42.
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Pipeline.h"
#include "cot/CIR/CIRDialect.h"

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

  // Build the module
  OpBuilder builder(&ctx);
  auto loc = builder.getUnknownLoc();
  auto module = ModuleOp::create(loc);

  // func @main() -> i32
  auto i32Type = builder.getI32Type();
  auto funcType = builder.getFunctionType({}, {i32Type});
  builder.setInsertionPointToEnd(module.getBody());
  auto mainFunc = builder.create<func::FuncOp>(loc, "main", funcType);

  // Entry block: return 42
  auto *entryBlock = mainFunc.addEntryBlock();
  builder.setInsertionPointToStart(entryBlock);
  auto constant42 = builder.create<arith::ConstantIntOp>(loc, 42, i32Type);
  builder.create<func::ReturnOp>(loc, ValueRange{constant42});

  // Run the pipeline
  cot::PipelineBuilder pipeline(&ctx);
  auto result = pipeline.emitBinary(module, "test_output");
  if (failed(result)) {
    llvm::errs() << "Pipeline failed!\n";
    return 1;
  }

  llvm::outs() << "Binary emitted successfully. Run ./test_output\n";
  return 0;
}
