//===- CIRCApi.cpp - CIR C API implementation -----------------*- C++ -*-===//
//
// C API builder functions for all Phase 2 CIR ops.
// Construct headers are conditionally included based on build-time discovery.
//
//===----------------------------------------------------------------------===//
#include "cot-c/CIRCApi.h"
#include "cot/CIR/CIRDialect.h"
#include "cot/Construct/Construct.h"

#ifdef COT_HAS_CORE
#include "cot-core/Ops.h"
#endif
#ifdef COT_HAS_MEMORY
#include "cot-memory/Types.h"
#include "cot-memory/Ops.h"
#endif
#ifdef COT_HAS_FLOW
#include "cot-flow/Ops.h"
#endif

#include "mlir/CAPI/IR.h"
#include "mlir/IR/Builders.h"

using namespace mlir;

// Helper: get an OpBuilder at the end of a block.
static OpBuilder getBuilderAtEnd(MlirBlock block) {
  Block *b = unwrap(block);
  return OpBuilder::atBlockEnd(b);
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

void cirBuildTrap(MlirBlock block, MlirLocation loc) {
  auto builder = getBuilderAtEnd(block);
  builder.create<cir::TrapOp>(unwrap(loc));
}

#endif // COT_HAS_FLOW
