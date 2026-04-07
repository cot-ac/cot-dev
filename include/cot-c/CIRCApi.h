//===- CIRCApi.h - CIR IR Building C API ----------------------*- C -*-===//
//
// C API for building CIR IR: creating ops, types, blocks, modules.
// Bindings wrap these functions — they are the stable ABI.
//
//===----------------------------------------------------------------------===//
#ifndef CIR_C_API_CIRCAPI_H
#define CIR_C_API_CIRCAPI_H

#include "mlir-c/IR.h"

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Dialect registration
//===----------------------------------------------------------------------===//

/// Register the CIR dialect with the context.
void cirRegisterDialect(MlirContext ctx);

//===----------------------------------------------------------------------===//
// Source locations (for error messages)
//===----------------------------------------------------------------------===//

/// Create a file:line:col source location.
MlirLocation cirLocationFileLineCol(MlirContext ctx,
                                    const char *filename,
                                    unsigned line, unsigned col);

//===----------------------------------------------------------------------===//
// Construct registration
//===----------------------------------------------------------------------===//

/// Register all linked construct ops/types with the context.
/// Call after cirRegisterDialect and before building any CIR ops.
void cirRegisterConstructs(MlirContext ctx);

//===----------------------------------------------------------------------===//
// cot-core: Constants
//===----------------------------------------------------------------------===//

MlirValue cirBuildConstantInt(MlirBlock block, MlirLocation loc,
                              MlirType type, int64_t value);
MlirValue cirBuildConstantFloat(MlirBlock block, MlirLocation loc,
                                MlirType type, double value);
MlirValue cirBuildConstantBool(MlirBlock block, MlirLocation loc,
                               bool value);

//===----------------------------------------------------------------------===//
// cot-core: Arithmetic
//===----------------------------------------------------------------------===//

MlirValue cirBuildAdd(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs);
MlirValue cirBuildSub(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs);
MlirValue cirBuildMul(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs);
MlirValue cirBuildDiv(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned);
MlirValue cirBuildRem(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned);
MlirValue cirBuildNeg(MlirBlock block, MlirLocation loc,
                      MlirValue operand);

//===----------------------------------------------------------------------===//
// cot-core: Comparison and Select
//===----------------------------------------------------------------------===//

/// Build cir.cmp (integer) or cir.cmpf (float) based on operand type.
/// Predicate is the enum value from CmpIPredicate or CmpFPredicate.
MlirValue cirBuildCmp(MlirBlock block, MlirLocation loc,
                      int64_t predicate, MlirValue lhs, MlirValue rhs);
MlirValue cirBuildSelect(MlirBlock block, MlirLocation loc,
                         MlirValue condition, MlirValue trueVal,
                         MlirValue falseVal);

//===----------------------------------------------------------------------===//
// cot-core: Bitwise
//===----------------------------------------------------------------------===//

MlirValue cirBuildBitAnd(MlirBlock block, MlirLocation loc,
                         MlirValue lhs, MlirValue rhs);
MlirValue cirBuildBitOr(MlirBlock block, MlirLocation loc,
                        MlirValue lhs, MlirValue rhs);
MlirValue cirBuildBitXor(MlirBlock block, MlirLocation loc,
                         MlirValue lhs, MlirValue rhs);
MlirValue cirBuildBitNot(MlirBlock block, MlirLocation loc,
                         MlirValue operand);
MlirValue cirBuildShl(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs);
MlirValue cirBuildShr(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned);

//===----------------------------------------------------------------------===//
// cot-core: Casts
//===----------------------------------------------------------------------===//

MlirValue cirBuildExtSI(MlirBlock block, MlirLocation loc,
                        MlirValue input, MlirType resultType);
MlirValue cirBuildExtUI(MlirBlock block, MlirLocation loc,
                        MlirValue input, MlirType resultType);
MlirValue cirBuildTruncI(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType);
MlirValue cirBuildSIToFP(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType);
MlirValue cirBuildFPToSI(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType);
MlirValue cirBuildExtF(MlirBlock block, MlirLocation loc,
                       MlirValue input, MlirType resultType);
MlirValue cirBuildTruncF(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType);

//===----------------------------------------------------------------------===//
// cot-memory: Types
//===----------------------------------------------------------------------===//

MlirType cirPointerTypeGet(MlirContext ctx);
MlirType cirRefTypeGet(MlirType pointeeType);

//===----------------------------------------------------------------------===//
// cot-memory: Ops
//===----------------------------------------------------------------------===//

MlirValue cirBuildAlloca(MlirBlock block, MlirLocation loc,
                         MlirType elemType);
void cirBuildStore(MlirBlock block, MlirLocation loc,
                   MlirValue value, MlirValue addr);
MlirValue cirBuildLoad(MlirBlock block, MlirLocation loc,
                       MlirValue addr, MlirType resultType);
MlirValue cirBuildAddrOf(MlirBlock block, MlirLocation loc,
                         MlirValue addr, MlirType refType);
MlirValue cirBuildDeref(MlirBlock block, MlirLocation loc,
                        MlirValue ref, MlirType resultType);

//===----------------------------------------------------------------------===//
// cot-flow: Ops
//===----------------------------------------------------------------------===//

void cirBuildBr(MlirBlock block, MlirLocation loc, MlirBlock dest);
void cirBuildCondBr(MlirBlock block, MlirLocation loc,
                    MlirValue condition,
                    MlirBlock trueDest, MlirBlock falseDest);
void cirBuildTrap(MlirBlock block, MlirLocation loc);

#ifdef __cplusplus
}
#endif

#endif // CIR_C_API_CIRCAPI_H
