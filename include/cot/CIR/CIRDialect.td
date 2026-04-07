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

  // Use generated type parser/printer for all CIR types
  let useDefaultTypePrinterParser = 1;

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

    /// Register static types from a construct.
    template <typename... Args>
    void registerConstructTypes() {
      addTypes<Args...>();
    }
  }];
}

#endif // CIR_DIALECT
