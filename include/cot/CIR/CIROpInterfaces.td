//===- CIROpInterfaces.td - CIR op interfaces -----------------*- tablegen -*-===//
//
// Op interfaces that CIR ops from any construct can implement.
// These live in the framework so constructs can query them without
// importing each other's code — all dependencies flow through cot-dev.
//
//===----------------------------------------------------------------------===//

#ifndef CIR_OP_INTERFACES
#define CIR_OP_INTERFACES

include "mlir/IR/OpBase.td"

//===----------------------------------------------------------------------===//
// ComptimeEvaluable — compile-time evaluation of ops
//===----------------------------------------------------------------------===//

def CIR_ComptimeEvaluable : OpInterface<"ComptimeEvaluable"> {
  let cppNamespace = "::cir";
  let description = [{
    Ops implementing this interface can be evaluated at compile time
    when all operands have comptime-known values (represented as Attributes).

    The CIRSema comptime step queries this interface generically:
    if an op implements ComptimeEvaluable and all operands are in the
    comptime value map, it calls comptimeEval() to produce a constant
    result and replaces the op.

    Pure ops (no side effects) implement comptimeEval().
    Side-effecting ops (memory simulation) implement comptimeExec().

    Reference: Zig Sema comptime evaluation, MLIR FoldOpInterface.
  }];

  let methods = [
    InterfaceMethod<
      /*desc=*/ [{
        Evaluate this op at compile time with constant inputs.
        Returns the constant result as an Attribute, or nullptr if
        evaluation is not possible for these inputs.

        Used by: arithmetic ops, comparison ops, casts, select, etc.
        Pure ops that produce a single result.
      }],
      /*retTy=*/ "mlir::Attribute",
      /*methodName=*/ "comptimeEval",
      /*args=*/ (ins "mlir::ArrayRef<mlir::Attribute>":$operands),
      /*methodBody=*/ "",
      /*defaultImplementation=*/ "return nullptr;"
    >,
    InterfaceMethod<
      /*desc=*/ [{
        Does this op have comptime-simulatable side effects?
        If true, the comptime step calls comptimeExec() instead of
        comptimeEval(). Used by: alloca, store, load (simulated memory),
        condbr (branch elimination).
      }],
      /*retTy=*/ "bool",
      /*methodName=*/ "hasComptimeSideEffects",
      /*args=*/ (ins),
      /*methodBody=*/ "",
      /*defaultImplementation=*/ "return false;"
    >,
  ];
}

#endif // CIR_OP_INTERFACES
