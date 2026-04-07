//===- CIROps.h - CIR op declarations -------------------------*- C++ -*-===//
#ifndef CIR_CIROPS_H
#define CIR_CIROPS_H

#include "cot/CIR/CIRDialect.h"
#include "cot/CIR/CIRTypes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/CastInterfaces.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

// Generated from CIROps.td
#define GET_OP_CLASSES
#include "cot/CIR/CIROps.h.inc"

#endif // CIR_CIROPS_H
