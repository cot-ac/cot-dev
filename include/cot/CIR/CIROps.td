//===- CIROps.td - Base classes for CIR ops -----------------*- tablegen -*-===//
//
// Base op classes used by all CIR constructs. No concrete ops — those go
// in construct repos (cot-core, cot-memory, etc.).
//
//===----------------------------------------------------------------------===//

#ifndef CIR_OPS_BASE
#define CIR_OPS_BASE

include "cot/CIR/CIRDialect.td"
include "mlir/IR/OpBase.td"
include "mlir/IR/EnumAttr.td"
include "mlir/Interfaces/CastInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"

/// Base class for all CIR ops.
/// Every CIR operation inherits from this. It sets the dialect to "cir".
class CIR_Op<string mnemonic, list<Trait> traits = []> :
    Op<CIR_Dialect, mnemonic, traits>;

/// Type constraint: integer types.
/// When cot-generics is added, this will also accept !cir.type_param<"T">.
def CIR_IntLike : TypeConstraint<
    CPred<"llvm::isa<IntegerType>($_self)">,
    "integer">;

/// Binary op on any type (int, float, or type_param).
/// Pure (no side effects), same operand/result type.
/// Used by: add, sub, mul
class CIR_BinaryOp<string mnemonic, list<Trait> traits = []> :
    CIR_Op<mnemonic, !listconcat([Pure, SameOperandsAndResultType], traits)> {
  let arguments = (ins AnyType:$lhs, AnyType:$rhs);
  let results = (outs AnyType:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

/// Binary op on integers or type_param. Pure, same operand/result type.
/// Used by: bit_and, bit_or, xor, shl, shr, divsi, divui, remsi, remui
class CIR_IntBinaryOp<string mnemonic, list<Trait> traits = []> :
    CIR_Op<mnemonic, !listconcat([Pure, SameOperandsAndResultType], traits)> {
  let arguments = (ins CIR_IntLike:$lhs, CIR_IntLike:$rhs);
  let results = (outs CIR_IntLike:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

/// Unary op on integers or type_param. Pure, same operand/result type.
/// Used by: neg, bit_not
class CIR_IntUnaryOp<string mnemonic, list<Trait> traits = []> :
    CIR_Op<mnemonic, !listconcat([Pure, SameOperandsAndResultType], traits)> {
  let arguments = (ins CIR_IntLike:$operand);
  let results = (outs CIR_IntLike:$result);
  let assemblyFormat = "$operand attr-dict `:` type($result)";
}

/// Binary op on floats. Pure, same operand/result type.
/// Used by: divf, remf
class CIR_FloatBinaryOp<string mnemonic, list<Trait> traits = []> :
    CIR_Op<mnemonic, !listconcat([Pure, SameOperandsAndResultType], traits)> {
  let arguments = (ins AnyFloat:$lhs, AnyFloat:$rhs);
  let results = (outs AnyFloat:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

/// Base class for cast ops. Pure, input -> output of different type.
/// Each cast maps 1:1 to one LLVM op. No if/else chains in lowering.
/// Reference: Arith_CastOp — CastOpInterface enables MLIR fold/verify.
class CIR_CastOp<string mnemonic, TypeConstraint From, TypeConstraint To,
                  list<Trait> traits = []> :
    CIR_Op<mnemonic, !listconcat([Pure,
      DeclareOpInterfaceMethods<CastOpInterface>], traits)> {
  let arguments = (ins From:$input);
  let results = (outs To:$result);
  let assemblyFormat = "$input attr-dict `:` type($input) `to` type($result)";
}

/// Specializations for each cast direction:
class CIR_IToICastOp<string mnemonic, list<Trait> traits = []> :
    CIR_CastOp<mnemonic, AnyInteger, AnyInteger, traits>;
class CIR_IToFCastOp<string mnemonic, list<Trait> traits = []> :
    CIR_CastOp<mnemonic, AnyInteger, AnyFloat, traits>;
class CIR_FToICastOp<string mnemonic, list<Trait> traits = []> :
    CIR_CastOp<mnemonic, AnyFloat, AnyInteger, traits>;
class CIR_FToFCastOp<string mnemonic, list<Trait> traits = []> :
    CIR_CastOp<mnemonic, AnyFloat, AnyFloat, traits>;

#endif // CIR_OPS_BASE
