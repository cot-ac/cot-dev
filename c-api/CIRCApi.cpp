//===- CIRCApi.cpp - CIR C API implementation -----------------*- C++ -*-===//
#include "cot-c/CIRCApi.h"
#include "cot/CIR/CIRDialect.h"

#include "mlir/CAPI/IR.h"

void cirRegisterDialect(MlirContext ctx) {
  unwrap(ctx)->getOrLoadDialect<cir::CIRDialect>();
}

MlirLocation cirLocationFileLineCol(MlirContext ctx,
                                    const char *filename,
                                    unsigned line, unsigned col) {
  return mlirLocationFileLineColGet(ctx, mlirStringRefCreate(
      filename, strlen(filename)), line, col);
}
