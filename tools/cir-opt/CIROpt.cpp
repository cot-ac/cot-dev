//===- CIROpt.cpp - CIR optimizer driver ----------------------*- C++ -*-===//
//
// Minimal MlirOptMain-based tool that loads CIR + Func + LLVM dialects
// and registers all passes. Required for lit tests.
//
//===----------------------------------------------------------------------===//
#include "cot/CIR/CIRDialect.h"
#include "cot/Construct/Construct.h"
#include "cot/Pipeline/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<cir::CIRDialect>();
  registry.insert<mlir::func::FuncDialect>();
  registry.insert<mlir::arith::ArithDialect>();
  registry.insert<mlir::LLVM::LLVMDialect>();

  // Register passes
  mlir::registerPass([]() -> std::unique_ptr<mlir::Pass> {
    return cot::createCIRToLLVMPass();
  });

  // Register construct ops/types from all linked constructs.
  // Constructs are linked via COT_REGISTER_CONSTRUCT static ctors.
  // We need to register their ops with a temporary context to make
  // them available for parsing.
  registry.addExtension(+[](mlir::MLIRContext *ctx,
                             cir::CIRDialect *dialect) {
    for (auto &construct : cot::getConstructRegistry())
      construct->registerOpsAndTypes(*ctx);
  });

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "CIR optimizer\n", registry));
}
