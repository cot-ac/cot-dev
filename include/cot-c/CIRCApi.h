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

/// Ensure construct libraries are not dead-stripped by the linker.
/// Call once from your program's main before cirRegisterConstructs.
/// This is needed when linking construct .a files without -force_load
/// (e.g., from Zig, Go, or other non-CMake build systems).
void cirForceConstructLink(void);

//===----------------------------------------------------------------------===//
// Block helpers
//===----------------------------------------------------------------------===//

/// Get the block argument at the given index as a Value.
MlirValue cirBlockGetArgument(MlirBlock block, intptr_t index);

//===----------------------------------------------------------------------===//
// Function building (func dialect)
//===----------------------------------------------------------------------===//

/// Create a FunctionType: (paramTypes...) -> (resultTypes...).
MlirType cirFunctionTypeGet(MlirContext ctx,
                            intptr_t numParams,
                            const MlirType *paramTypes,
                            intptr_t numResults,
                            const MlirType *resultTypes);

/// Get the number of input parameters in a FunctionType.
intptr_t cirFunctionTypeGetNumInputs(MlirType funcType);

/// Get the number of results in a FunctionType.
intptr_t cirFunctionTypeGetNumResults(MlirType funcType);

/// Get the parameter type at the given index.
MlirType cirFunctionTypeGetInput(MlirType funcType, intptr_t index);

/// Get the result type at the given index.
MlirType cirFunctionTypeGetResult(MlirType funcType, intptr_t index);

/// Create a func.func operation with a body region containing entryBlock.
/// The entry block defines the function's parameter types via its arguments.
/// Returns the created func.func operation.
MlirOperation cirFuncCreate(MlirBlock moduleBody, MlirLocation loc,
                             const char *name, MlirType funcType,
                             MlirBlock entryBlock);

/// Get the body region of a func.func operation.
/// Use to append additional blocks to the function.
MlirRegion cirFuncGetBodyRegion(MlirOperation funcOp);

/// Build a func.call operation.
/// Returns the first result value, or a null MlirValue for void calls.
MlirValue cirFuncCall(MlirBlock block, MlirLocation loc,
                      const char *callee,
                      intptr_t numArgs, const MlirValue *args,
                      intptr_t numResults, const MlirType *resultTypes);

/// Build a func.return operation (0 or more return values).
void cirFuncReturn(MlirBlock block, MlirLocation loc,
                   intptr_t numValues, const MlirValue *values);

/// Look up the return type of a named function in the module.
/// Returns a null MlirType if the function is not found or returns void.
MlirType cirFuncLookupReturnType(MlirOperation moduleOp,
                                 const char *funcName);

/// Check if a named function returns void (0 results).
/// Returns true if the function returns void or is not found.
bool cirFuncIsVoidReturn(MlirOperation moduleOp, const char *funcName);

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

/// Get the pointee type of a ref type: !cir.ref<T> -> T.
MlirType cirRefTypeGetPointee(MlirType refType);

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

/// Build cir.switch — multi-way branch on integer value.
/// Branches to the case destination matching `value`, or to `defaultDest`
/// if no case matches. caseValues and caseDests arrays must have
/// numCases elements.
void cirBuildSwitch(MlirBlock block, MlirLocation loc,
                    MlirValue value,
                    MlirBlock defaultDest,
                    intptr_t numCases,
                    const int64_t *caseValues,
                    const MlirBlock *caseDests);

void cirBuildTrap(MlirBlock block, MlirLocation loc);

//===----------------------------------------------------------------------===//
// cot-structs: Types
//===----------------------------------------------------------------------===//

/// Create a struct type: !cir.struct<"name", "f0": T0, "f1": T1, ...>.
/// fieldNames and fieldTypes arrays must have numFields elements.
MlirType cirStructTypeGet(MlirContext ctx, const char *name,
                          intptr_t numFields,
                          const char *const *fieldNames,
                          const MlirType *fieldTypes);

/// Get the struct name.
MlirStringRef cirStructTypeGetName(MlirType structType);

/// Get the number of fields in a struct type.
intptr_t cirStructTypeGetNumFields(MlirType structType);

/// Get the name of field at index. Caller does NOT own the string data.
MlirStringRef cirStructTypeGetFieldName(MlirType structType,
                                        intptr_t index);

/// Get the type of field at index.
MlirType cirStructTypeGetFieldType(MlirType structType,
                                   intptr_t index);

//===----------------------------------------------------------------------===//
// cot-structs: Ops
//===----------------------------------------------------------------------===//

/// Build cir.struct_init from field values. Returns the struct value.
MlirValue cirBuildStructInit(MlirBlock block, MlirLocation loc,
                             MlirType structType,
                             intptr_t numFields,
                             const MlirValue *fields);

/// Build cir.field_val — extract field at index from struct value.
MlirValue cirBuildFieldVal(MlirBlock block, MlirLocation loc,
                           MlirType resultType,
                           MlirValue input, int64_t index);

/// Build cir.field_ptr — pointer to field at index in struct at base addr.
MlirValue cirBuildFieldPtr(MlirBlock block, MlirLocation loc,
                           MlirType resultType,
                           MlirValue base, int64_t index,
                           MlirType structType);

//===----------------------------------------------------------------------===//
// cot-arrays: Types
//===----------------------------------------------------------------------===//

/// Create an array type: !cir.array<size x elementType>.
MlirType cirArrayTypeGet(MlirContext ctx, int64_t size,
                         MlirType elementType);

/// Get the fixed size of an array type.
int64_t cirArrayTypeGetSize(MlirType arrayType);

/// Get the element type of an array type.
MlirType cirArrayTypeGetElementType(MlirType arrayType);

//===----------------------------------------------------------------------===//
// cot-arrays: Ops
//===----------------------------------------------------------------------===//

/// Build cir.array_init from element values. Returns the array value.
MlirValue cirBuildArrayInit(MlirBlock block, MlirLocation loc,
                            MlirType arrayType,
                            intptr_t numElements,
                            const MlirValue *elements);

/// Build cir.elem_val — extract element at constant index from array.
MlirValue cirBuildElemVal(MlirBlock block, MlirLocation loc,
                          MlirType resultType,
                          MlirValue input, int64_t index);

/// Build cir.elem_ptr — pointer to element at dynamic index in array.
MlirValue cirBuildElemPtr(MlirBlock block, MlirLocation loc,
                          MlirType resultType,
                          MlirValue base, MlirValue index,
                          MlirType arrayType);

//===----------------------------------------------------------------------===//
// cot-slices: Types
//===----------------------------------------------------------------------===//

/// Create a slice type: !cir.slice<elementType>.
MlirType cirSliceTypeGet(MlirContext ctx, MlirType elementType);

/// Get the element type of a slice type.
MlirType cirSliceTypeGetElementType(MlirType sliceType);

//===----------------------------------------------------------------------===//
// cot-slices: Ops
//===----------------------------------------------------------------------===//

/// Build cir.string_constant — create a slice from a string literal.
MlirValue cirBuildStringConstant(MlirBlock block, MlirLocation loc,
                                 MlirType sliceType,
                                 const char *value);

/// Build cir.slice_ptr — extract data pointer from slice.
MlirValue cirBuildSlicePtr(MlirBlock block, MlirLocation loc,
                           MlirType resultType, MlirValue input);

/// Build cir.slice_len — extract length (i64) from slice.
MlirValue cirBuildSliceLen(MlirBlock block, MlirLocation loc,
                           MlirValue input);

/// Build cir.slice_elem — load element at index from slice.
MlirValue cirBuildSliceElem(MlirBlock block, MlirLocation loc,
                            MlirType resultType,
                            MlirValue input, MlirValue index);

/// Build cir.array_to_slice — create slice from base ptr + start/end.
MlirValue cirBuildArrayToSlice(MlirBlock block, MlirLocation loc,
                               MlirType sliceType,
                               MlirValue base,
                               MlirValue start, MlirValue end);

//===----------------------------------------------------------------------===//
// cot-optionals: Types
//===----------------------------------------------------------------------===//

/// Create an optional type: !cir.optional<payloadType>.
MlirType cirOptionalTypeGet(MlirContext ctx, MlirType payloadType);

/// Get the payload type from an optional type.
MlirType cirOptionalTypeGetPayload(MlirType optionalType);

//===----------------------------------------------------------------------===//
// cot-optionals: Ops
//===----------------------------------------------------------------------===//

/// Build cir.none — create a null optional.
MlirValue cirBuildNone(MlirBlock block, MlirLocation loc,
                       MlirType optionalType);

/// Build cir.wrap_optional — wrap a value into an optional.
MlirValue cirBuildWrapOptional(MlirBlock block, MlirLocation loc,
                               MlirType optionalType, MlirValue input);

/// Build cir.is_non_null — check if optional has a value (returns i1).
MlirValue cirBuildIsNonNull(MlirBlock block, MlirLocation loc,
                            MlirValue input);

/// Build cir.optional_payload — extract payload from non-null optional.
MlirValue cirBuildOptionalPayload(MlirBlock block, MlirLocation loc,
                                  MlirType resultType, MlirValue input);

//===----------------------------------------------------------------------===//
// cot-errors: Types
//===----------------------------------------------------------------------===//

/// Create an error union type: !cir.error_union<payloadType>.
MlirType cirErrorUnionTypeGet(MlirContext ctx, MlirType payloadType);

/// Get the payload type from an error union type.
MlirType cirErrorUnionTypeGetPayload(MlirType errorUnionType);

//===----------------------------------------------------------------------===//
// cot-errors: Ops
//===----------------------------------------------------------------------===//

/// Build cir.wrap_result — wrap success value into error union.
MlirValue cirBuildWrapResult(MlirBlock block, MlirLocation loc,
                             MlirType errorUnionType, MlirValue input);

/// Build cir.wrap_error — wrap error code (i16) into error union.
MlirValue cirBuildWrapError(MlirBlock block, MlirLocation loc,
                            MlirType errorUnionType, MlirValue code);

/// Build cir.is_error — check if error union is error (returns i1).
MlirValue cirBuildIsError(MlirBlock block, MlirLocation loc,
                          MlirValue input);

/// Build cir.error_payload — extract payload from success error union.
MlirValue cirBuildErrorPayload(MlirBlock block, MlirLocation loc,
                               MlirType resultType, MlirValue input);

/// Build cir.error_code — extract error code (i16) from error union.
MlirValue cirBuildErrorCode(MlirBlock block, MlirLocation loc,
                            MlirValue input);

//===----------------------------------------------------------------------===//
// cot-enums: Types
//===----------------------------------------------------------------------===//

/// Create an enum type: !cir.enum<"name", tagType, "V0", "V1", ...>.
/// variants is an array of numVariants null-terminated strings.
MlirType cirEnumTypeGet(MlirContext ctx, const char *name,
                        MlirType tagType,
                        intptr_t numVariants,
                        const char *const *variants);

/// Get the number of variants in an enum type.
intptr_t cirEnumTypeGetVariantCount(MlirType enumType);

/// Get the tag type of an enum type.
MlirType cirEnumTypeGetTagType(MlirType enumType);

/// Get the name of an enum type.
MlirStringRef cirEnumTypeGetName(MlirType enumType);

/// Get the name of variant at index.
MlirStringRef cirEnumTypeGetVariantName(MlirType enumType,
                                        intptr_t index);

//===----------------------------------------------------------------------===//
// cot-enums: Ops
//===----------------------------------------------------------------------===//

/// Build cir.enum_constant — produce enum value from variant name.
MlirValue cirBuildEnumConstant(MlirBlock block, MlirLocation loc,
                               MlirType enumType, const char *variant);

/// Build cir.enum_value — extract integer tag from enum.
MlirValue cirBuildEnumValue(MlirBlock block, MlirLocation loc,
                            MlirType resultType, MlirValue input);

//===----------------------------------------------------------------------===//
// cot-unions: Types
//===----------------------------------------------------------------------===//

/// Create a tagged union type:
///   !cir.tagged_union<"name", "V0": T0, "V1": T1, ...>.
/// variantNames and variantTypes arrays must have numVariants elements.
MlirType cirTaggedUnionTypeGet(MlirContext ctx, const char *name,
                               intptr_t numVariants,
                               const char *const *variantNames,
                               const MlirType *variantTypes);

/// Get the name of a tagged union type.
MlirStringRef cirTaggedUnionTypeGetName(MlirType unionType);

/// Get the number of variants in a tagged union type.
intptr_t cirTaggedUnionTypeGetNumVariants(MlirType unionType);

/// Get the name of variant at index.
MlirStringRef cirTaggedUnionTypeGetVariantName(MlirType unionType,
                                               intptr_t index);

/// Get the type of variant at index.
MlirType cirTaggedUnionTypeGetVariantType(MlirType unionType,
                                          intptr_t index);

//===----------------------------------------------------------------------===//
// cot-unions: Ops
//===----------------------------------------------------------------------===//

/// Build cir.union_init — create tagged union with variant and payload.
/// payload may be a null MlirValue for payload-less variants.
MlirValue cirBuildUnionInit(MlirBlock block, MlirLocation loc,
                            MlirType unionType,
                            const char *variant, MlirValue payload);

/// Build cir.union_tag — extract i8 discriminator tag.
MlirValue cirBuildUnionTag(MlirBlock block, MlirLocation loc,
                           MlirValue input);

/// Build cir.union_payload — extract payload for a specific variant.
MlirValue cirBuildUnionPayload(MlirBlock block, MlirLocation loc,
                               MlirType resultType,
                               const char *variant, MlirValue input);

//===----------------------------------------------------------------------===//
// cot-test: Ops
//===----------------------------------------------------------------------===//

/// Build cir.assert — runtime assertion with diagnostic message.
void cirBuildAssert(MlirBlock block, MlirLocation loc,
                    MlirValue condition, const char *message);

/// Build cir.test_case — named test case region op.
/// Returns the operation (caller must populate the body region).
MlirOperation cirBuildTestCase(MlirBlock block, MlirLocation loc,
                               const char *name);

//===----------------------------------------------------------------------===//
// Type inspectors
//===----------------------------------------------------------------------===//

bool cirTypeIsPtr(MlirType type);
bool cirTypeIsRef(MlirType type);
bool cirTypeIsStruct(MlirType type);
bool cirTypeIsArray(MlirType type);
bool cirTypeIsSlice(MlirType type);
bool cirTypeIsOptional(MlirType type);
bool cirTypeIsErrorUnion(MlirType type);
bool cirTypeIsEnum(MlirType type);
bool cirTypeIsTaggedUnion(MlirType type);

//===----------------------------------------------------------------------===//
// Operation inspection — CIR-specific helpers for transform authors
//===----------------------------------------------------------------------===//

/// Check if an operation has a specific op name (e.g., "cir.add").
bool cirOperationIsA(MlirOperation op, const char *opName);

/// Get the result type of an operation's first result.
MlirType cirOperationGetResultType(MlirOperation op);

/// Get the type of an operation's operand at the given index.
MlirType cirOperationGetOperandType(MlirOperation op, intptr_t index);

/// Get a string attribute value from an operation by name.
/// Returns a null MlirStringRef if the attribute doesn't exist or isn't a
/// string. Caller does NOT own the returned string data.
MlirStringRef cirOperationGetStringAttr(MlirOperation op,
                                        const char *attrName);

/// Get an integer attribute value from an operation by name.
/// Returns 0 if the attribute doesn't exist or isn't an integer.
int64_t cirOperationGetIntAttr(MlirOperation op, const char *attrName);

#ifdef __cplusplus
}
#endif

#endif // CIR_C_API_CIRCAPI_H
