//===- CIRDialect.td - CIR dialect definition ----------------*- tablegen -*-===//
//
// Defines the CIR (Cot Intermediate Representation) MLIR dialect.
// CIR inherits ExtensibleDialect for runtime op/type registration.
//
//===----------------------------------------------------------------------===//

#ifndef CIR_DIALECT
#define CIR_DIALECT

include "mlir/IR/OpBase.td"

def CIR_Dialect : Dialect {
  let name = "cir";
  let summary = "Cot Intermediate Representation — universal IR for C-style languages";
  let cppNamespace = "::cir";

  // Enable runtime extension: constructs register new ops via plugins
  let isExtensible = 1;

  // Type parsing/printing is handled by CIRDialect::parseType/printType
  // which delegates to registered constructs. NOT using the generated
  // default because construct repos define types in separate compilation
  // units — the generated code would try to define CIRDialect methods
  // in each construct's .cpp.inc.
  let useDefaultTypePrinterParser = 0;

  // Required for constant folding across blocks — creates cir.constant
  // when a constant value needs to be rematerialized in a new block
  let hasConstantMaterializer = 1;

  let dependentDialects = [
    "mlir::func::FuncDialect",
    "mlir::LLVM::LLVMDialect",
    "mlir::DLTIDialect"
  ];

  let extraClassDeclaration = [{
    /// Register a dynamic op from a construct plugin.
    /// Called by COT_REGISTER_CONSTRUCT during plugin loading.
    void registerConstructOp(
        std::unique_ptr<mlir::DynamicOpDefinition> op);

    /// Register a dynamic type from a construct plugin.
    void registerConstructType(
        std::unique_ptr<mlir::DynamicTypeDefinition> type);

    /// Register static ops from a construct (e.g., cot-core).
    /// Wraps the protected addOperations<>().
    template <typename... Args>
    void registerConstructOps() {
      addOperations<Args...>();
    }

    /// Register static types from a construct (e.g., cot-memory).
    /// Wraps the protected addTypes<>().
    template <typename... Args>
    void registerConstructTypes() {
      addTypes<Args...>();
    }

    /// Parse a CIR type — delegates to registered constructs,
    /// then falls back to dynamic types.
    ::mlir::Type parseType(::mlir::DialectAsmParser &parser) const override;

    /// Print a CIR type — delegates to registered constructs,
    /// then falls back to dynamic types.
    void printType(::mlir::Type type,
                   ::mlir::DialectAsmPrinter &printer) const override;
  }];
}

#endif // CIR_DIALECT
