//===- CIRToLLVM.cpp - CIR to LLVM lowering pass --------------*- C++ -*-===//
//
// The lowering pass that constructs populate with patterns.
// Errata E2: TypeConverter needs source/target materializations.
// Errata E14: ReconcileUnrealizedCastsPass added after this pass.
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Passes.h"
#include "cot/Construct/Construct.h"
#include "cot/CIR/CIRDialect.h"

#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace {

struct CIRToLLVMPass
    : public PassWrapper<CIRToLLVMPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CIRToLLVMPass)

  StringRef getArgument() const override { return "cir-to-llvm"; }
  StringRef getDescription() const override {
    return "Lower CIR to LLVM dialect";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<LLVM::LLVMDialect>();
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *ctx = &getContext();

    // Create TypeConverter with construct-provided type conversions
    LLVMTypeConverter typeConverter(ctx);

    // Add type conversions from all registered constructs
    for (auto &construct : cot::getConstructRegistry())
      construct->addTypeConversions(typeConverter);

    // Source/target materializations (errata E2)
    typeConverter.addSourceMaterialization(
        [](OpBuilder &builder, Type resultType, ValueRange inputs,
           Location loc) -> Value {
          if (inputs.size() != 1)
            return Value();
          return builder
              .create<UnrealizedConversionCastOp>(loc, resultType,
                                                  inputs)
              .getResult(0);
        });
    typeConverter.addTargetMaterialization(
        [](OpBuilder &builder, Type resultType, ValueRange inputs,
           Location loc) -> Value {
          if (inputs.size() != 1)
            return Value();
          return builder
              .create<UnrealizedConversionCastOp>(loc, resultType,
                                                  inputs)
              .getResult(0);
        });

    // Create ConversionTarget — LLVM legal, CIR illegal
    ConversionTarget target(*ctx);
    target.addLegalDialect<LLVM::LLVMDialect>();
    target.addIllegalDialect<cir::CIRDialect>();
    target.addLegalOp<ModuleOp>();

    // Collect lowering patterns from all constructs
    RewritePatternSet patterns(ctx);
    for (auto &construct : cot::getConstructRegistry())
      construct->populateLoweringPatterns(patterns, typeConverter);

    // Standard dialect->LLVM patterns
    arith::populateArithToLLVMConversionPatterns(typeConverter,
                                                    patterns);
    populateFuncToLLVMConversionPatterns(typeConverter, patterns);

    // Apply full conversion
    if (failed(applyFullConversion(module, target,
                                   std::move(patterns))))
      signalPassFailure();
  }
};

} // anonymous namespace

std::unique_ptr<Pass> cot::createCIRToLLVMPass() {
  return std::make_unique<CIRToLLVMPass>();
}
