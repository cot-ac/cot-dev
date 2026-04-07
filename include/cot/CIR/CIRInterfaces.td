//===- CIRInterfaces.td - CIR type interfaces ----------------*- tablegen -*-===//
//
// Interfaces that CIR types from any construct can implement.
// These live in the framework so constructs can query them without
// importing each other's code — all dependencies flow through cot-dev.
//
//===----------------------------------------------------------------------===//

#ifndef CIR_INTERFACES
#define CIR_INTERFACES

include "mlir/IR/OpBase.td"

def CIR_PointerLikeTypeInterface
    : TypeInterface<"PointerLikeTypeInterface"> {
  let cppNamespace = "::cir";
  let description = [{
    Marker interface for CIR types that lower to LLVM opaque pointers.

    Types implementing this interface are eligible for null-pointer
    optimization in `!cir.optional<T>`: when T is pointer-like, the
    optional is represented as a bare pointer (null = none) instead
    of a struct<(T, i1)>.

    Implemented by: !cir.ptr, !cir.ref<T>, and future types like
    !cir.box<T>, !cir.unowned<T>, !cir.weak<T>.
  }];
}

#endif // CIR_INTERFACES
