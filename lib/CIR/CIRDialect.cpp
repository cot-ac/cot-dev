//===- CIRDialect.cpp - CIR dialect implementation ----------*- C++ -*-===//
#include "cot/CIR/CIRDialect.h"
#include "cot/CIR/CIROps.h"
#include "cot/CIR/CIRTypes.h"

#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace cir;

// TableGen-generated implementations
#include "cot/CIR/CIRDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "cot/CIR/CIRTypes.cpp.inc"

#define GET_OP_CLASSES
#include "cot/CIR/CIROps.cpp.inc"

void CIRDialect::initialize() {
  // No statically-defined ops or types in the framework.
  // Concrete ops/types are defined in construct repos (cot-core, etc.)
  // and registered via registerConstructOp/registerConstructType.
}

// Type parsing/printing — no CIR types in the framework yet.
// When construct repos add types (e.g., !cir.ptr, !cir.optional),
// they will be registered as dynamic types.
Type CIRDialect::parseType(DialectAsmParser &parser) const {
  StringRef keyword;
  if (parser.parseKeyword(&keyword))
    return Type();

  // Try parsing as a dynamic type (from construct plugins)
  Type resultType;
  auto parseResult =
      parseOptionalDynamicType(keyword, parser, resultType);
  if (parseResult.has_value()) {
    if (succeeded(*parseResult))
      return resultType;
    return Type();
  }

  parser.emitError(parser.getNameLoc(), "unknown CIR type: ")
      << keyword;
  return Type();
}

void CIRDialect::printType(Type type,
                            DialectAsmPrinter &printer) const {
  // Try printing as a dynamic type
  if (succeeded(printIfDynamicType(type, printer)))
    return;
  llvm_unreachable("unknown CIR type");
}

// Constant materializer (errata E1).
// In Phase 1, no ConstantOp exists yet (it's defined in cot-core).
// Return nullptr — constant folding across blocks won't work until
// cot-core is linked, which provides ConstantOp.
Operation *CIRDialect::materializeConstant(
    OpBuilder &builder, Attribute value, Type type, Location loc) {
  return nullptr;
}

void CIRDialect::registerConstructOp(
    std::unique_ptr<DynamicOpDefinition> op) {
  registerDynamicOp(std::move(op));
}

void CIRDialect::registerConstructType(
    std::unique_ptr<DynamicTypeDefinition> type) {
  registerDynamicType(std::move(type));
}
