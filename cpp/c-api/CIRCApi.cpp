//===- CIRCApi.cpp - CIR C API implementation -----------------*- C++ -*-===//
//
// C API builder functions for all core CIR ops.
// Construct headers are conditionally included based on build-time discovery.
//
//===----------------------------------------------------------------------===//
#include "cot-c/CIRCApi.h"
#include "cot/CIR/CIRDialect.h"
#include "cot/Construct/Construct.h"

#ifdef COT_HAS_CORE
#include "arith/Ops.h"
#endif
#ifdef COT_HAS_MEMORY
#include "memory/Types.h"
#include "memory/Ops.h"
#endif
#ifdef COT_HAS_FLOW
#include "flow/Ops.h"
#endif
#ifdef COT_HAS_STRUCTS
#include "structs/Types.h"
#include "structs/Ops.h"
#endif
#ifdef COT_HAS_ARRAYS
#include "arrays/Types.h"
#include "arrays/Ops.h"
#endif
#ifdef COT_HAS_SLICES
#include "slices/Types.h"
#include "slices/Ops.h"
#endif
#ifdef COT_HAS_OPTIONALS
#include "optionals/Types.h"
#include "optionals/Ops.h"
#endif
#ifdef COT_HAS_ERRORS
#include "errors/Types.h"
#include "errors/Ops.h"
#endif
#ifdef COT_HAS_ENUMS
#include "enums/Types.h"
#include "enums/Ops.h"
#endif
#ifdef COT_HAS_UNIONS
#include "unions/Types.h"
#include "unions/Ops.h"
#endif
#ifdef COT_HAS_TEST
#include "test/Ops.h"
#endif
#ifdef COT_HAS_GENERICS
#include "generics/Types.h"
#include "generics/Ops.h"
#endif
#ifdef COT_HAS_TRAITS
#include "traits/Types.h"
#include "traits/Ops.h"
#endif
#ifdef COT_HAS_VWT
#include "vwt/Ops.h"
#endif

#include "mlir/CAPI/IR.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir-c/BuiltinAttributes.h"

using namespace mlir;

// Helper: get an OpBuilder at the end of a block.
static OpBuilder getBuilderAtEnd(MlirBlock block) {
  Block *b = unwrap(block);
  return OpBuilder::atBlockEnd(b);
}

//===----------------------------------------------------------------------===//
// Block helpers
//===----------------------------------------------------------------------===//

MlirValue cirBlockGetArgument(MlirBlock block, intptr_t index) {
  return wrap(unwrap(block)->getArgument(index));
}

//===----------------------------------------------------------------------===//
// Function building (func dialect)
//===----------------------------------------------------------------------===//

MlirType cirFunctionTypeGet(MlirContext ctx,
                            intptr_t numParams,
                            const MlirType *paramTypes,
                            intptr_t numResults,
                            const MlirType *resultTypes) {
  SmallVector<Type> params, results;
  for (intptr_t i = 0; i < numParams; ++i)
    params.push_back(unwrap(paramTypes[i]));
  for (intptr_t i = 0; i < numResults; ++i)
    results.push_back(unwrap(resultTypes[i]));
  return wrap(FunctionType::get(unwrap(ctx), params, results));
}

intptr_t cirFunctionTypeGetNumInputs(MlirType funcType) {
  return mlir::cast<FunctionType>(unwrap(funcType)).getNumInputs();
}

intptr_t cirFunctionTypeGetNumResults(MlirType funcType) {
  return mlir::cast<FunctionType>(unwrap(funcType)).getNumResults();
}

MlirType cirFunctionTypeGetInput(MlirType funcType, intptr_t index) {
  return wrap(mlir::cast<FunctionType>(unwrap(funcType)).getInput(index));
}

MlirType cirFunctionTypeGetResult(MlirType funcType, intptr_t index) {
  return wrap(mlir::cast<FunctionType>(unwrap(funcType)).getResult(index));
}

MlirOperation cirFuncCreate(MlirBlock moduleBody, MlirLocation loc,
                             const char *name, MlirType funcType,
                             MlirBlock entryBlock) {
  auto builder = getBuilderAtEnd(moduleBody);
  auto location = unwrap(loc);
  auto fty = mlir::cast<FunctionType>(unwrap(funcType));

  // Create the function op
  auto funcOp = builder.create<func::FuncOp>(location, name, fty);

  // Move entry block into the function's body region
  funcOp.getBody().push_back(unwrap(entryBlock));

  return wrap(funcOp.getOperation());
}

MlirRegion cirFuncGetBodyRegion(MlirOperation funcOp) {
  auto fn = mlir::cast<func::FuncOp>(unwrap(funcOp));
  return wrap(&fn.getBody());
}

MlirValue cirFuncCall(MlirBlock block, MlirLocation loc,
                      const char *callee,
                      intptr_t numArgs, const MlirValue *args,
                      intptr_t numResults, const MlirType *resultTypes) {
  auto builder = getBuilderAtEnd(block);
  auto location = unwrap(loc);

  SmallVector<Value> argVals;
  for (intptr_t i = 0; i < numArgs; ++i)
    argVals.push_back(unwrap(args[i]));

  SmallVector<Type> resTys;
  for (intptr_t i = 0; i < numResults; ++i)
    resTys.push_back(unwrap(resultTypes[i]));

  auto callOp = builder.create<func::CallOp>(location, callee, resTys,
                                              argVals);
  if (numResults > 0)
    return wrap(callOp.getResult(0));
  return {nullptr};
}

void cirFuncReturn(MlirBlock block, MlirLocation loc,
                   intptr_t numValues, const MlirValue *values) {
  auto builder = getBuilderAtEnd(block);
  SmallVector<Value> vals;
  for (intptr_t i = 0; i < numValues; ++i)
    vals.push_back(unwrap(values[i]));
  builder.create<func::ReturnOp>(unwrap(loc), vals);
}

MlirType cirFuncLookupReturnType(MlirOperation moduleOp,
                                 const char *funcName) {
  auto mod = unwrap(moduleOp);
  SymbolTable symTable(mod);
  auto funcOp = symTable.lookup<func::FuncOp>(funcName);
  if (!funcOp || funcOp.getNumResults() == 0)
    return {nullptr};
  return wrap(funcOp.getResultTypes()[0]);
}

bool cirFuncIsVoidReturn(MlirOperation moduleOp, const char *funcName) {
  auto mod = unwrap(moduleOp);
  SymbolTable symTable(mod);
  auto funcOp = symTable.lookup<func::FuncOp>(funcName);
  return !funcOp || funcOp.getNumResults() == 0;
}

//===----------------------------------------------------------------------===//
// Dialect + Construct registration
//===----------------------------------------------------------------------===//

void cirRegisterDialect(MlirContext ctx) {
  unwrap(ctx)->getOrLoadDialect<cir::CIRDialect>();
}

MlirLocation cirLocationFileLineCol(MlirContext ctx,
                                    const char *filename,
                                    unsigned line, unsigned col) {
  return mlirLocationFileLineColGet(ctx, mlirStringRefCreate(
      filename, strlen(filename)), line, col);
}

void cirRegisterConstructs(MlirContext ctx) {
  auto *context = unwrap(ctx);
  for (auto &construct : cot::getConstructRegistry())
    construct->registerOpsAndTypes(*context);
}

// C-linkage anchor functions emitted by COT_REGISTER_CONSTRUCT.
// Each construct's registration .cpp gets one of these. Referencing
// them here forces the linker to pull in the .o file that contains
// the static constructor — needed when not using -force_load.
extern "C" {
#ifdef COT_HAS_CORE
void _cot_anchor_CoreConstruct(void);
#endif
#ifdef COT_HAS_MEMORY
void _cot_anchor_MemoryConstruct(void);
#endif
#ifdef COT_HAS_FLOW
void _cot_anchor_FlowConstruct(void);
#endif
#ifdef COT_HAS_STRUCTS
void _cot_anchor_StructsConstruct(void);
#endif
#ifdef COT_HAS_ARRAYS
void _cot_anchor_ArraysConstruct(void);
#endif
#ifdef COT_HAS_SLICES
void _cot_anchor_SlicesConstruct(void);
#endif
#ifdef COT_HAS_OPTIONALS
void _cot_anchor_OptionalsConstruct(void);
#endif
#ifdef COT_HAS_ERRORS
void _cot_anchor_ErrorsConstruct(void);
#endif
#ifdef COT_HAS_TEST
void _cot_anchor_TestConstruct(void);
#endif
#ifdef COT_HAS_ENUMS
void _cot_anchor_EnumsConstruct(void);
#endif
#ifdef COT_HAS_UNIONS
void _cot_anchor_UnionsConstruct(void);
#endif
#ifdef COT_HAS_GENERICS
void _cot_anchor_GenericsConstruct(void);
#endif
#ifdef COT_HAS_TRAITS
void _cot_anchor_TraitsConstruct(void);
#endif
#ifdef COT_HAS_VWT
void _cot_anchor_VWTConstruct(void);
#endif
#ifdef COT_HAS_COMPTIME
void _cot_anchor_ComptimeConstruct(void);
#endif
} // extern "C"

void cirForceConstructLink() {
  // Call each anchor — the calls are trivial (empty functions) but
  // they force the linker to keep the translation units alive.
#ifdef COT_HAS_CORE
  _cot_anchor_CoreConstruct();
#endif
#ifdef COT_HAS_MEMORY
  _cot_anchor_MemoryConstruct();
#endif
#ifdef COT_HAS_FLOW
  _cot_anchor_FlowConstruct();
#endif
#ifdef COT_HAS_STRUCTS
  _cot_anchor_StructsConstruct();
#endif
#ifdef COT_HAS_ARRAYS
  _cot_anchor_ArraysConstruct();
#endif
#ifdef COT_HAS_SLICES
  _cot_anchor_SlicesConstruct();
#endif
#ifdef COT_HAS_OPTIONALS
  _cot_anchor_OptionalsConstruct();
#endif
#ifdef COT_HAS_ERRORS
  _cot_anchor_ErrorsConstruct();
#endif
#ifdef COT_HAS_TEST
  _cot_anchor_TestConstruct();
#endif
#ifdef COT_HAS_ENUMS
  _cot_anchor_EnumsConstruct();
#endif
#ifdef COT_HAS_UNIONS
  _cot_anchor_UnionsConstruct();
#endif
#ifdef COT_HAS_GENERICS
  _cot_anchor_GenericsConstruct();
#endif
#ifdef COT_HAS_TRAITS
  _cot_anchor_TraitsConstruct();
#endif
#ifdef COT_HAS_VWT
  _cot_anchor_VWTConstruct();
#endif
#ifdef COT_HAS_COMPTIME
  _cot_anchor_ComptimeConstruct();
#endif
}

//===----------------------------------------------------------------------===//
// cot-core: Constants
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_CORE

MlirValue cirBuildConstantInt(MlirBlock block, MlirLocation loc,
                              MlirType type, int64_t value) {
  auto builder = getBuilderAtEnd(block);
  auto t = unwrap(type);
  auto attr = builder.getIntegerAttr(t, value);
  return wrap(builder.create<cir::ConstantOp>(unwrap(loc), t, attr)
                  .getResult());
}

MlirValue cirBuildConstantFloat(MlirBlock block, MlirLocation loc,
                                MlirType type, double value) {
  auto builder = getBuilderAtEnd(block);
  auto t = unwrap(type);
  auto attr = builder.getFloatAttr(t, value);
  return wrap(builder.create<cir::ConstantOp>(unwrap(loc), t, attr)
                  .getResult());
}

MlirValue cirBuildConstantBool(MlirBlock block, MlirLocation loc,
                               bool value) {
  auto builder = getBuilderAtEnd(block);
  auto t = builder.getI1Type();
  auto attr = builder.getBoolAttr(value);
  return wrap(builder.create<cir::ConstantOp>(unwrap(loc), t, attr)
                  .getResult());
}

//===----------------------------------------------------------------------===//
// cot-core: Arithmetic
//===----------------------------------------------------------------------===//

MlirValue cirBuildAdd(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::AddOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildSub(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::SubOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildMul(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::MulOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildDiv(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  auto location = unwrap(loc);
  auto type = l.getType();
  if (mlir::isa<FloatType>(type))
    return wrap(builder.create<cir::DivFOp>(location, type, l, r)
                    .getResult());
  if (isSigned)
    return wrap(builder.create<cir::DivSIOp>(location, type, l, r)
                    .getResult());
  return wrap(builder.create<cir::DivUIOp>(location, type, l, r)
                  .getResult());
}

MlirValue cirBuildRem(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  auto location = unwrap(loc);
  auto type = l.getType();
  if (mlir::isa<FloatType>(type))
    return wrap(builder.create<cir::RemFOp>(location, type, l, r)
                    .getResult());
  if (isSigned)
    return wrap(builder.create<cir::RemSIOp>(location, type, l, r)
                    .getResult());
  return wrap(builder.create<cir::RemUIOp>(location, type, l, r)
                  .getResult());
}

MlirValue cirBuildNeg(MlirBlock block, MlirLocation loc,
                      MlirValue operand) {
  auto builder = getBuilderAtEnd(block);
  auto v = unwrap(operand);
  auto location = unwrap(loc);
  auto type = v.getType();
  if (mlir::isa<FloatType>(type))
    return wrap(builder.create<cir::NegFOp>(location, type, v)
                    .getResult());
  return wrap(builder.create<cir::NegOp>(location, type, v)
                  .getResult());
}

//===----------------------------------------------------------------------===//
// cot-core: Comparison and Select
//===----------------------------------------------------------------------===//

MlirValue cirBuildCmp(MlirBlock block, MlirLocation loc,
                      int64_t predicate, MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  auto location = unwrap(loc);
  auto i1Type = builder.getI1Type();
  if (mlir::isa<FloatType>(l.getType())) {
    auto pred = static_cast<cir::CmpFPredicate>(predicate);
    auto predAttr = cir::CmpFPredicateAttr::get(builder.getContext(), pred);
    return wrap(builder.create<cir::CmpFOp>(location, i1Type, predAttr, l, r)
                    .getResult());
  }
  auto pred = static_cast<cir::CmpIPredicate>(predicate);
  auto predAttr = cir::CmpIPredicateAttr::get(builder.getContext(), pred);
  return wrap(builder.create<cir::CmpOp>(location, i1Type, predAttr, l, r)
                  .getResult());
}

MlirValue cirBuildSelect(MlirBlock block, MlirLocation loc,
                         MlirValue condition, MlirValue trueVal,
                         MlirValue falseVal) {
  auto builder = getBuilderAtEnd(block);
  auto c = unwrap(condition), t = unwrap(trueVal), f = unwrap(falseVal);
  return wrap(builder.create<cir::SelectOp>(unwrap(loc), t.getType(), c, t, f)
                  .getResult());
}

//===----------------------------------------------------------------------===//
// cot-core: Bitwise
//===----------------------------------------------------------------------===//

MlirValue cirBuildBitAnd(MlirBlock block, MlirLocation loc,
                         MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::BitAndOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildBitOr(MlirBlock block, MlirLocation loc,
                        MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::BitOrOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildBitXor(MlirBlock block, MlirLocation loc,
                         MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::BitXorOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildBitNot(MlirBlock block, MlirLocation loc,
                         MlirValue operand) {
  auto builder = getBuilderAtEnd(block);
  auto v = unwrap(operand);
  return wrap(builder.create<cir::BitNotOp>(unwrap(loc), v.getType(), v)
                  .getResult());
}

MlirValue cirBuildShl(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  return wrap(builder.create<cir::ShlOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

MlirValue cirBuildShr(MlirBlock block, MlirLocation loc,
                      MlirValue lhs, MlirValue rhs, bool isSigned) {
  auto builder = getBuilderAtEnd(block);
  auto l = unwrap(lhs), r = unwrap(rhs);
  if (isSigned)
    return wrap(builder.create<cir::ShrSOp>(unwrap(loc), l.getType(), l, r)
                    .getResult());
  return wrap(builder.create<cir::ShrOp>(unwrap(loc), l.getType(), l, r)
                  .getResult());
}

//===----------------------------------------------------------------------===//
// cot-core: Casts
//===----------------------------------------------------------------------===//

MlirValue cirBuildExtSI(MlirBlock block, MlirLocation loc,
                        MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ExtSIOp>(unwrap(loc), unwrap(resultType),
                                            unwrap(input))
                  .getResult());
}

MlirValue cirBuildExtUI(MlirBlock block, MlirLocation loc,
                        MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ExtUIOp>(unwrap(loc), unwrap(resultType),
                                            unwrap(input))
                  .getResult());
}

MlirValue cirBuildTruncI(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::TruncIOp>(unwrap(loc), unwrap(resultType),
                                             unwrap(input))
                  .getResult());
}

MlirValue cirBuildSIToFP(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::SIToFPOp>(unwrap(loc), unwrap(resultType),
                                             unwrap(input))
                  .getResult());
}

MlirValue cirBuildFPToSI(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::FPToSIOp>(unwrap(loc), unwrap(resultType),
                                             unwrap(input))
                  .getResult());
}

MlirValue cirBuildExtF(MlirBlock block, MlirLocation loc,
                       MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ExtFOp>(unwrap(loc), unwrap(resultType),
                                           unwrap(input))
                  .getResult());
}

MlirValue cirBuildTruncF(MlirBlock block, MlirLocation loc,
                         MlirValue input, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::TruncFOp>(unwrap(loc), unwrap(resultType),
                                             unwrap(input))
                  .getResult());
}

#endif // COT_HAS_CORE

//===----------------------------------------------------------------------===//
// cot-memory: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_MEMORY

MlirType cirPointerTypeGet(MlirContext ctx) {
  return wrap(cir::PointerType::get(unwrap(ctx)));
}

MlirType cirRefTypeGet(MlirType pointeeType) {
  auto pt = unwrap(pointeeType);
  return wrap(cir::RefType::get(pt.getContext(), pt));
}

MlirType cirRefTypeGetPointee(MlirType refType) {
  return wrap(mlir::cast<cir::RefType>(unwrap(refType)).getPointeeType());
}

MlirValue cirBuildAlloca(MlirBlock block, MlirLocation loc,
                         MlirType elemType) {
  auto builder = getBuilderAtEnd(block);
  auto ctx = unwrap(loc).getContext();
  auto ptrType = cir::PointerType::get(ctx);
  return wrap(builder.create<cir::AllocaOp>(
                         unwrap(loc), ptrType,
                         TypeAttr::get(unwrap(elemType)))
                  .getResult());
}

void cirBuildStore(MlirBlock block, MlirLocation loc,
                   MlirValue value, MlirValue addr) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::StoreOp>(unwrap(loc), unwrap(value), unwrap(addr));
}

MlirValue cirBuildLoad(MlirBlock block, MlirLocation loc,
                       MlirValue addr, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::LoadOp>(unwrap(loc), unwrap(resultType),
                                           unwrap(addr))
                  .getResult());
}

MlirValue cirBuildAddrOf(MlirBlock block, MlirLocation loc,
                         MlirValue addr, MlirType refType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::AddrOfOp>(
                         unwrap(loc),
                         mlir::cast<cir::RefType>(unwrap(refType)),
                         unwrap(addr))
                  .getResult());
}

MlirValue cirBuildDeref(MlirBlock block, MlirLocation loc,
                        MlirValue ref, MlirType resultType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::DerefOp>(unwrap(loc), unwrap(resultType),
                                            unwrap(ref))
                  .getResult());
}

#endif // COT_HAS_MEMORY

//===----------------------------------------------------------------------===//
// cot-flow: Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_FLOW

void cirBuildBr(MlirBlock block, MlirLocation loc, MlirBlock dest) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::BrOp>(unwrap(loc), ValueRange{}, unwrap(dest));
}

void cirBuildCondBr(MlirBlock block, MlirLocation loc,
                    MlirValue condition,
                    MlirBlock trueDest, MlirBlock falseDest) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::CondBrOp>(unwrap(loc), unwrap(condition),
                                 ValueRange{}, ValueRange{},
                                 unwrap(trueDest), unwrap(falseDest));
}

void cirBuildSwitch(MlirBlock block, MlirLocation loc,
                    MlirValue value,
                    MlirBlock defaultDest,
                    intptr_t numCases,
                    const int64_t *caseValues,
                    const MlirBlock *caseDests) {
  auto builder = getBuilderAtEnd(block);
  SmallVector<Block *> destBlocks;
  for (intptr_t i = 0; i < numCases; ++i)
    destBlocks.push_back(unwrap(caseDests[i]));
  auto caseAttr = builder.getDenseI64ArrayAttr(
      ArrayRef<int64_t>(caseValues, numCases));
  builder.create<cir::SwitchOp>(unwrap(loc), unwrap(value), caseAttr,
                                 unwrap(defaultDest), destBlocks);
}

void cirBuildTrap(MlirBlock block, MlirLocation loc) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::TrapOp>(unwrap(loc));
}

#endif // COT_HAS_FLOW

//===----------------------------------------------------------------------===//
// cot-structs: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_STRUCTS

MlirType cirStructTypeGet(MlirContext ctx, const char *name,
                          intptr_t numFields,
                          const char *const *fieldNames,
                          const MlirType *fieldTypes) {
  auto *context = unwrap(ctx);
  SmallVector<StringAttr> names;
  SmallVector<Type> types;
  for (intptr_t i = 0; i < numFields; i++) {
    names.push_back(StringAttr::get(context, fieldNames[i]));
    types.push_back(unwrap(fieldTypes[i]));
  }
  return wrap(cir::StructType::get(
      context, StringAttr::get(context, name), names, types));
}

MlirStringRef cirStructTypeGetName(MlirType structType) {
  auto sty = mlir::cast<cir::StructType>(unwrap(structType));
  auto name = sty.getName();
  return mlirStringRefCreate(name.data(), name.size());
}

intptr_t cirStructTypeGetNumFields(MlirType structType) {
  return mlir::cast<cir::StructType>(unwrap(structType))
      .getFieldTypes().size();
}

MlirStringRef cirStructTypeGetFieldName(MlirType structType,
                                        intptr_t index) {
  auto sty = mlir::cast<cir::StructType>(unwrap(structType));
  auto name = sty.getFieldNames()[index].getValue();
  return mlirStringRefCreate(name.data(), name.size());
}

MlirType cirStructTypeGetFieldType(MlirType structType,
                                   intptr_t index) {
  return wrap(mlir::cast<cir::StructType>(unwrap(structType))
                  .getFieldTypes()[index]);
}

MlirValue cirBuildStructInit(MlirBlock block, MlirLocation loc,
                             MlirType structType,
                             intptr_t numFields,
                             const MlirValue *fields) {
  auto builder = getBuilderAtEnd(block);
  SmallVector<Value> fieldVals;
  for (intptr_t i = 0; i < numFields; i++)
    fieldVals.push_back(unwrap(fields[i]));
  return wrap(builder.create<cir::StructInitOp>(
                         unwrap(loc), unwrap(structType), fieldVals)
                  .getResult());
}

MlirValue cirBuildFieldVal(MlirBlock block, MlirLocation loc,
                           MlirType resultType,
                           MlirValue input, int64_t index) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::FieldValOp>(
                         unwrap(loc), unwrap(resultType),
                         unwrap(input), index)
                  .getResult());
}

MlirValue cirBuildFieldPtr(MlirBlock block, MlirLocation loc,
                           MlirType resultType,
                           MlirValue base, int64_t index,
                           MlirType structType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::FieldPtrOp>(
                         unwrap(loc), unwrap(resultType),
                         unwrap(base), static_cast<uint64_t>(index),
                         unwrap(structType))
                  .getResult());
}

#endif // COT_HAS_STRUCTS

//===----------------------------------------------------------------------===//
// cot-arrays: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_ARRAYS

MlirType cirArrayTypeGet(MlirContext ctx, int64_t size,
                         MlirType elementType) {
  return wrap(cir::ArrayType::get(unwrap(ctx), size,
                                  unwrap(elementType)));
}

int64_t cirArrayTypeGetSize(MlirType arrayType) {
  return mlir::cast<cir::ArrayType>(unwrap(arrayType)).getSize();
}

MlirType cirArrayTypeGetElementType(MlirType arrayType) {
  return wrap(mlir::cast<cir::ArrayType>(unwrap(arrayType))
                  .getElementType());
}

MlirValue cirBuildArrayInit(MlirBlock block, MlirLocation loc,
                            MlirType arrayType,
                            intptr_t numElements,
                            const MlirValue *elements) {
  auto builder = getBuilderAtEnd(block);
  SmallVector<Value> elems;
  for (intptr_t i = 0; i < numElements; i++)
    elems.push_back(unwrap(elements[i]));
  return wrap(builder.create<cir::ArrayInitOp>(
                         unwrap(loc), unwrap(arrayType), elems)
                  .getResult());
}

MlirValue cirBuildElemVal(MlirBlock block, MlirLocation loc,
                          MlirType resultType,
                          MlirValue input, int64_t index) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ElemValOp>(
                         unwrap(loc), unwrap(resultType),
                         unwrap(input), index)
                  .getResult());
}

MlirValue cirBuildElemPtr(MlirBlock block, MlirLocation loc,
                          MlirType resultType,
                          MlirValue base, MlirValue index,
                          MlirType arrayType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ElemPtrOp>(
                         unwrap(loc), unwrap(resultType),
                         unwrap(base), unwrap(index),
                         TypeAttr::get(unwrap(arrayType)))
                  .getResult());
}

#endif // COT_HAS_ARRAYS

//===----------------------------------------------------------------------===//
// cot-slices: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_SLICES

MlirType cirSliceTypeGet(MlirContext ctx, MlirType elementType) {
  return wrap(cir::SliceType::get(unwrap(ctx), unwrap(elementType)));
}

MlirType cirSliceTypeGetElementType(MlirType sliceType) {
  return wrap(mlir::cast<cir::SliceType>(unwrap(sliceType))
                  .getElementType());
}

MlirValue cirBuildStringConstant(MlirBlock block, MlirLocation loc,
                                 MlirType sliceType,
                                 const char *value) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::StringConstantOp>(
                         unwrap(loc), unwrap(sliceType), value)
                  .getResult());
}

MlirValue cirBuildSlicePtr(MlirBlock block, MlirLocation loc,
                           MlirType resultType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::SlicePtrOp>(
                         unwrap(loc), unwrap(resultType), unwrap(input))
                  .getResult());
}

MlirValue cirBuildSliceLen(MlirBlock block, MlirLocation loc,
                           MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::SliceLenOp>(
                         unwrap(loc), builder.getI64Type(), unwrap(input))
                  .getResult());
}

MlirValue cirBuildSliceElem(MlirBlock block, MlirLocation loc,
                            MlirType resultType,
                            MlirValue input, MlirValue index) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::SliceElemOp>(
                         unwrap(loc), unwrap(resultType),
                         unwrap(input), unwrap(index))
                  .getResult());
}

MlirValue cirBuildArrayToSlice(MlirBlock block, MlirLocation loc,
                               MlirType sliceType,
                               MlirValue base,
                               MlirValue start, MlirValue end) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ArrayToSliceOp>(
                         unwrap(loc), unwrap(sliceType),
                         unwrap(base), unwrap(start), unwrap(end))
                  .getResult());
}

#endif // COT_HAS_SLICES

//===----------------------------------------------------------------------===//
// cot-optionals: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_OPTIONALS

MlirType cirOptionalTypeGet(MlirContext ctx, MlirType payloadType) {
  return wrap(cir::OptionalType::get(unwrap(ctx),
                                     unwrap(payloadType)));
}

MlirType cirOptionalTypeGetPayload(MlirType optionalType) {
  return wrap(mlir::cast<cir::OptionalType>(unwrap(optionalType))
                  .getPayloadType());
}

MlirValue cirBuildNone(MlirBlock block, MlirLocation loc,
                       MlirType optionalType) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::NoneOp>(unwrap(loc),
                                          unwrap(optionalType))
                  .getResult());
}

MlirValue cirBuildWrapOptional(MlirBlock block, MlirLocation loc,
                               MlirType optionalType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::WrapOptionalOp>(
                         unwrap(loc), unwrap(optionalType), unwrap(input))
                  .getResult());
}

MlirValue cirBuildIsNonNull(MlirBlock block, MlirLocation loc,
                            MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::IsNonNullOp>(
                         unwrap(loc), builder.getI1Type(), unwrap(input))
                  .getResult());
}

MlirValue cirBuildOptionalPayload(MlirBlock block, MlirLocation loc,
                                  MlirType resultType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::OptionalPayloadOp>(
                         unwrap(loc), unwrap(resultType), unwrap(input))
                  .getResult());
}

#endif // COT_HAS_OPTIONALS

//===----------------------------------------------------------------------===//
// cot-errors: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_ERRORS

MlirType cirErrorUnionTypeGet(MlirContext ctx, MlirType payloadType) {
  return wrap(cir::ErrorUnionType::get(unwrap(ctx),
                                       unwrap(payloadType)));
}

MlirType cirErrorUnionTypeGetPayload(MlirType errorUnionType) {
  return wrap(mlir::cast<cir::ErrorUnionType>(unwrap(errorUnionType))
                  .getPayloadType());
}

MlirValue cirBuildWrapResult(MlirBlock block, MlirLocation loc,
                             MlirType errorUnionType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::WrapResultOp>(
                         unwrap(loc), unwrap(errorUnionType), unwrap(input))
                  .getResult());
}

MlirValue cirBuildWrapError(MlirBlock block, MlirLocation loc,
                            MlirType errorUnionType, MlirValue code) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::WrapErrorOp>(
                         unwrap(loc), unwrap(errorUnionType), unwrap(code))
                  .getResult());
}

MlirValue cirBuildIsError(MlirBlock block, MlirLocation loc,
                          MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::IsErrorOp>(
                         unwrap(loc), builder.getI1Type(), unwrap(input))
                  .getResult());
}

MlirValue cirBuildErrorPayload(MlirBlock block, MlirLocation loc,
                               MlirType resultType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ErrorPayloadOp>(
                         unwrap(loc), unwrap(resultType), unwrap(input))
                  .getResult());
}

MlirValue cirBuildErrorCode(MlirBlock block, MlirLocation loc,
                            MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::ErrorCodeOp>(
                         unwrap(loc), builder.getIntegerType(16),
                         unwrap(input))
                  .getResult());
}

#endif // COT_HAS_ERRORS

//===----------------------------------------------------------------------===//
// cot-enums: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_ENUMS

MlirType cirEnumTypeGet(MlirContext ctx, const char *name,
                        MlirType tagType,
                        intptr_t numVariants,
                        const char *const *variants) {
  auto *context = unwrap(ctx);
  SmallVector<StringAttr> variantAttrs;
  for (intptr_t i = 0; i < numVariants; i++)
    variantAttrs.push_back(StringAttr::get(context, variants[i]));
  return wrap(cir::EnumType::get(context, StringRef(name),
                                 unwrap(tagType), variantAttrs));
}

intptr_t cirEnumTypeGetVariantCount(MlirType enumType) {
  return mlir::cast<cir::EnumType>(unwrap(enumType))
      .getVariants().size();
}

MlirType cirEnumTypeGetTagType(MlirType enumType) {
  return wrap(mlir::cast<cir::EnumType>(unwrap(enumType))
                  .getTagType());
}

MlirStringRef cirEnumTypeGetName(MlirType enumType) {
  auto name = mlir::cast<cir::EnumType>(unwrap(enumType)).getName();
  return mlirStringRefCreate(name.data(), name.size());
}

MlirStringRef cirEnumTypeGetVariantName(MlirType enumType,
                                        intptr_t index) {
  auto v = mlir::cast<cir::EnumType>(unwrap(enumType))
               .getVariants()[index].getValue();
  return mlirStringRefCreate(v.data(), v.size());
}

MlirValue cirBuildEnumConstant(MlirBlock block, MlirLocation loc,
                               MlirType enumType, const char *variant) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::EnumConstantOp>(
                         unwrap(loc), unwrap(enumType), variant)
                  .getResult());
}

MlirValue cirBuildEnumValue(MlirBlock block, MlirLocation loc,
                            MlirType resultType, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::EnumValueOp>(
                         unwrap(loc), unwrap(resultType), unwrap(input))
                  .getResult());
}

#endif // COT_HAS_ENUMS

//===----------------------------------------------------------------------===//
// cot-unions: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_UNIONS

MlirType cirTaggedUnionTypeGet(MlirContext ctx, const char *name,
                               intptr_t numVariants,
                               const char *const *variantNames,
                               const MlirType *variantTypes) {
  auto *context = unwrap(ctx);
  SmallVector<StringAttr> names;
  SmallVector<Type> types;
  for (intptr_t i = 0; i < numVariants; i++) {
    names.push_back(StringAttr::get(context, variantNames[i]));
    types.push_back(unwrap(variantTypes[i]));
  }
  return wrap(cir::TaggedUnionType::get(context, StringRef(name),
                                        names, types));
}

MlirStringRef cirTaggedUnionTypeGetName(MlirType unionType) {
  auto name = mlir::cast<cir::TaggedUnionType>(unwrap(unionType))
                  .getName();
  return mlirStringRefCreate(name.data(), name.size());
}

intptr_t cirTaggedUnionTypeGetNumVariants(MlirType unionType) {
  return mlir::cast<cir::TaggedUnionType>(unwrap(unionType))
      .getVariantNames().size();
}

MlirStringRef cirTaggedUnionTypeGetVariantName(MlirType unionType,
                                               intptr_t index) {
  auto v = mlir::cast<cir::TaggedUnionType>(unwrap(unionType))
               .getVariantNames()[index].getValue();
  return mlirStringRefCreate(v.data(), v.size());
}

MlirType cirTaggedUnionTypeGetVariantType(MlirType unionType,
                                          intptr_t index) {
  return wrap(mlir::cast<cir::TaggedUnionType>(unwrap(unionType))
                  .getVariantTypes()[index]);
}

MlirValue cirBuildUnionInit(MlirBlock block, MlirLocation loc,
                            MlirType unionType,
                            const char *variant, MlirValue payload) {
  auto builder = getBuilderAtEnd(block);
  Value payloadVal = payload.ptr ? unwrap(payload) : Value();
  return wrap(builder.create<cir::UnionInitOp>(
                         unwrap(loc), unwrap(unionType),
                         variant, payloadVal)
                  .getResult());
}

MlirValue cirBuildUnionTag(MlirBlock block, MlirLocation loc,
                           MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::UnionTagOp>(
                         unwrap(loc), builder.getIntegerType(8),
                         unwrap(input))
                  .getResult());
}

MlirValue cirBuildUnionPayload(MlirBlock block, MlirLocation loc,
                               MlirType resultType,
                               const char *variant, MlirValue input) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::UnionPayloadOp>(
                         unwrap(loc), unwrap(resultType),
                         variant, unwrap(input))
                  .getResult());
}

#endif // COT_HAS_UNIONS

//===----------------------------------------------------------------------===//
// cot-test: Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_TEST

void cirBuildAssert(MlirBlock block, MlirLocation loc,
                    MlirValue condition, const char *message) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::AssertOp>(
      unwrap(loc), unwrap(condition),
      builder.getStringAttr(message));
}

MlirOperation cirBuildTestCase(MlirBlock block, MlirLocation loc,
                               const char *name) {
  auto builder = getBuilderAtEnd(block);
  return wrap(builder.create<cir::TestCaseOp>(unwrap(loc), name)
                  .getOperation());
}

#endif // COT_HAS_TEST

//===----------------------------------------------------------------------===//
// cot-generics: Types and Ops
//===----------------------------------------------------------------------===//

#ifdef COT_HAS_GENERICS

MlirType cirTypeParamTypeGet(MlirContext ctx, const char *name) {
  return wrap(cir::TypeParamType::get(unwrap(ctx), name));
}

MlirStringRef cirTypeParamTypeGetName(MlirType typeParamType) {
  auto tp = mlir::cast<cir::TypeParamType>(unwrap(typeParamType));
  auto name = tp.getName();
  return {name.data(), name.size()};
}

MlirValue cirBuildGenericApply(MlirBlock block, MlirLocation loc,
                               const char *callee,
                               intptr_t numArgs, const MlirValue *args,
                               intptr_t numSubs,
                               const char *const *subsKeys,
                               const MlirType *subsTypes,
                               intptr_t numResults,
                               const MlirType *resultTypes) {
  auto builder = getBuilderAtEnd(block);
  auto location = unwrap(loc);
  auto ctx = builder.getContext();

  SmallVector<Value> argVals;
  for (intptr_t i = 0; i < numArgs; ++i)
    argVals.push_back(unwrap(args[i]));

  SmallVector<Attribute> keys, types;
  for (intptr_t i = 0; i < numSubs; ++i) {
    keys.push_back(StringAttr::get(ctx, subsKeys[i]));
    types.push_back(TypeAttr::get(unwrap(subsTypes[i])));
  }

  SmallVector<Type> resTys;
  for (intptr_t i = 0; i < numResults; ++i)
    resTys.push_back(unwrap(resultTypes[i]));

  auto calleeAttr = FlatSymbolRefAttr::get(ctx, callee);
  auto op = builder.create<cir::GenericApplyOp>(
      location, resTys, calleeAttr, argVals,
      ArrayAttr::get(ctx, keys), ArrayAttr::get(ctx, types));
  if (numResults > 0)
    return wrap(op.getResult(0));
  return {nullptr};
}

bool cirTypeIsTypeParam(MlirType type) {
  return mlir::isa<cir::TypeParamType>(unwrap(type));
}

#else

MlirType cirTypeParamTypeGet(MlirContext, const char *) { return {nullptr}; }
MlirStringRef cirTypeParamTypeGetName(MlirType) { return {nullptr, 0}; }
MlirValue cirBuildGenericApply(MlirBlock, MlirLocation, const char *,
                               intptr_t, const MlirValue *, intptr_t,
                               const char *const *, const MlirType *,
                               intptr_t, const MlirType *) {
  return {nullptr};
}
bool cirTypeIsTypeParam(MlirType) { return false; }

#endif // COT_HAS_GENERICS

//===----------------------------------------------------------------------===//
// Type inspectors
//===----------------------------------------------------------------------===//

bool cirTypeIsPtr(MlirType type) {
#ifdef COT_HAS_MEMORY
  return mlir::isa<cir::PointerType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsRef(MlirType type) {
#ifdef COT_HAS_MEMORY
  return mlir::isa<cir::RefType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsStruct(MlirType type) {
#ifdef COT_HAS_STRUCTS
  return mlir::isa<cir::StructType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsArray(MlirType type) {
#ifdef COT_HAS_ARRAYS
  return mlir::isa<cir::ArrayType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsSlice(MlirType type) {
#ifdef COT_HAS_SLICES
  return mlir::isa<cir::SliceType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsOptional(MlirType type) {
#ifdef COT_HAS_OPTIONALS
  return mlir::isa<cir::OptionalType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsErrorUnion(MlirType type) {
#ifdef COT_HAS_ERRORS
  return mlir::isa<cir::ErrorUnionType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsEnum(MlirType type) {
#ifdef COT_HAS_ENUMS
  return mlir::isa<cir::EnumType>(unwrap(type));
#else
  return false;
#endif
}

bool cirTypeIsTaggedUnion(MlirType type) {
#ifdef COT_HAS_UNIONS
  return mlir::isa<cir::TaggedUnionType>(unwrap(type));
#else
  return false;
#endif
}

//===----------------------------------------------------------------------===//
// Operation inspection — CIR-specific helpers for transform authors
//===----------------------------------------------------------------------===//

bool cirOperationIsA(MlirOperation op, const char *opName) {
  auto name = mlirIdentifierStr(mlirOperationGetName(op));
  return StringRef(name.data, name.length) == opName;
}

MlirType cirOperationGetResultType(MlirOperation op) {
  return mlirValueGetType(mlirOperationGetResult(op, 0));
}

MlirType cirOperationGetOperandType(MlirOperation op, intptr_t index) {
  return mlirValueGetType(mlirOperationGetOperand(op, index));
}

MlirStringRef cirOperationGetStringAttr(MlirOperation op,
                                        const char *attrName) {
  MlirAttribute attr = mlirOperationGetAttributeByName(
      op, mlirStringRefCreateFromCString(attrName));
  if (mlirAttributeIsNull(attr) || !mlirAttributeIsAString(attr)) {
    MlirStringRef empty = {nullptr, 0};
    return empty;
  }
  return mlirStringAttrGetValue(attr);
}

int64_t cirOperationGetIntAttr(MlirOperation op, const char *attrName) {
  MlirAttribute attr = mlirOperationGetAttributeByName(
      op, mlirStringRefCreateFromCString(attrName));
  if (mlirAttributeIsNull(attr) || !mlirAttributeIsAInteger(attr))
    return 0;
  return mlirIntegerAttrGetValueSInt(attr);
}
