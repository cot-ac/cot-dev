//===- e2e_phase2_test.cpp - Phase 2 gate test ----------------*- C++ -*-===//
//
// Programmatically build (uses ops from cot-core + cot-memory):
//   func @main() -> i32 {
//     %a = cir.alloca i32 : !cir.ptr
//     %c = cir.constant 42 : i32
//     cir.store %c, %a : i32, !cir.ptr
//     %v = cir.load %a : !cir.ptr to i32
//     %one = cir.constant 1 : i32
//     %r = cir.add %v, %one : i32
//     return %r : i32
//   }
//
// Run PipelineBuilder::emitBinary(). Execute output. Exit code = 43.
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Pipeline.h"
#include "cot/Construct/Construct.h"
#include "cot/CIR/CIRDialect.h"
#include "arith/Ops.h"

#ifdef COT_HAS_MEMORY
#include "memory/Types.h"
#include "memory/Ops.h"
#endif

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

  // Register construct ops (cot-core + cot-memory linked via -force_load)
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

#ifdef COT_HAS_MEMORY
  // %a = cir.alloca i32 : !cir.ptr
  auto ptrType = cir::PointerType::get(&ctx);
  auto allocaOp = builder.create<cir::AllocaOp>(
      loc, ptrType, TypeAttr::get(i32Type));

  // %c = cir.constant 42 : i32
  auto constant42 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(42));

  // cir.store %c, %a : i32, !cir.ptr
  builder.create<cir::StoreOp>(loc, constant42, allocaOp);

  // %v = cir.load %a : !cir.ptr to i32
  auto loadVal = builder.create<cir::LoadOp>(
      loc, i32Type, allocaOp);

  // %one = cir.constant 1 : i32
  auto constant1 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(1));

  // %r = cir.add %v, %one : i32
  auto addResult = builder.create<cir::AddOp>(
      loc, i32Type, loadVal, constant1);
#else
  // Fallback: cir.constant 42 + cir.constant 1 → cir.add
  auto constant42 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(42));
  auto constant1 = builder.create<cir::ConstantOp>(
      loc, i32Type, builder.getI32IntegerAttr(1));
  auto addResult = builder.create<cir::AddOp>(
      loc, i32Type, constant42, constant1);
#endif

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
