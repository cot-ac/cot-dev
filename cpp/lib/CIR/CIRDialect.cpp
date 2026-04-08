//===- CIRDialect.cpp - CIR dialect implementation ----------*- C++ -*-===//
#include "cot/CIR/CIRDialect.h"
#include "cot/CIR/CIRInterfaces.h"
#include "cot/CIR/CIROpInterfaces.h"
#include "cot/CIR/CIROps.h"
#include "cot/CIR/CIRTypes.h"
#include "cot/Construct/Construct.h"

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

// TypeInterface implementations
#include "cot/CIR/CIRInterfaces.cpp.inc"

// OpInterface implementations
#include "cot/CIR/CIROpInterfaces.cpp.inc"

void CIRDialect::initialize() {
  // No statically-defined ops or types in the framework.
  // Concrete ops/types are defined in construct repos (cot-core, etc.)
  // and registered via registerConstructOp/registerConstructType.
}

// Type parsing/printing — delegates to registered constructs, then
// falls back to dynamic types (ExtensibleDialect plugins).
Type CIRDialect::parseType(DialectAsmParser &parser) const {
  StringRef keyword;
  if (parser.parseKeyword(&keyword))
    return Type();

  // Try construct type parsers (static types from cot-memory, etc.)
  for (auto &construct : cot::getConstructRegistry()) {
    Type result;
    auto parseResult = construct->parseType(keyword, parser, result);
    if (parseResult.has_value()) {
      if (succeeded(*parseResult))
        return result;
      return Type();
    }
  }

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
  // Try construct type printers (static types from cot-memory, etc.)
  for (auto &construct : cot::getConstructRegistry()) {
    if (succeeded(construct->printType(type, printer)))
      return;
  }

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
