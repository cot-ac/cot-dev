//===- CIRTypes.td - Base classes for CIR types -------------*- tablegen -*-===//
//
// Base type class used by all CIR constructs. No concrete types — those go
// in construct repos (cot-memory, cot-structs, etc.).
//
//===----------------------------------------------------------------------===//

#ifndef CIR_TYPES_BASE
#define CIR_TYPES_BASE

include "cot/CIR/CIRDialect.td"
include "mlir/IR/AttrTypeBase.td"

/// Base class for all CIR types.
class CIR_Type<string name, string typeMnemonic, list<Trait> traits = []> :
    TypeDef<CIR_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

#endif // CIR_TYPES_BASE
