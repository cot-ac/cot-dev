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

#ifdef __cplusplus
}
#endif

#endif // CIR_C_API_CIRCAPI_H
