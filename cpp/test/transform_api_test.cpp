//===- transform_api_test.cpp - Transform C API test -----------*- C++ -*-===//
//
// Validates that external passes created via cotCreateFuncPass work.
//
// 1. Build CIR programmatically via C API:
//      func @main() -> i32 {
//        %a = cir.constant 10 : i32
//        %b = cir.constant 3 : i32
//        %r = cir.add %a, %b : i32       // 10 + 3 = 13
//        return %r : i32
//      }
//
// 2. Create an external pass via cotCreateFuncPass that walks CIR and
//    replaces every cir.add with cir.sub.
//
// 3. Run the pipeline with the pass in the postSema slot.
//
// 4. Emit binary. Execute it. Exit code should be 7 (10 - 3), proving
//    the external transform ran and modified the IR.
//
//===----------------------------------------------------------------------===//
#include "cot-c/CIRCApi.h"
#include "cot-c/COTCApi.h"
#include "mlir-c/IR.h"
#include "mlir-c/BuiltinTypes.h"
#include "mlir-c/BuiltinAttributes.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// The transform: replace every cir.add with cir.sub.
// This is the callback that runs on each func — written purely via C API.
static MlirWalkResult replaceAddWithSub(MlirOperation op, void *) {
  if (!cirOperationIsA(op, "cir.add"))
    return MlirWalkResultAdvance;

  // Get operands and result type
  MlirValue lhs = mlirOperationGetOperand(op, 0);
  MlirValue rhs = mlirOperationGetOperand(op, 1);
  MlirType resultType = mlirValueGetType(mlirOperationGetResult(op, 0));
  MlirLocation loc = mlirOperationGetLocation(op);

  // Build a cir.sub at the same position by inserting before this op.
  // We use the block + insertBefore pattern via the raw MLIR C API.
  MlirBlock block = mlirOperationGetBlock(op);

  // Build the replacement op using mlirOperationCreate
  MlirOperationState state =
      mlirOperationStateGet(mlirStringRefCreateFromCString("cir.sub"), loc);
  mlirOperationStateAddOperands(&state, 1, &lhs);
  mlirOperationStateAddOperands(&state, 1, &rhs);
  mlirOperationStateAddResults(&state, 1, &resultType);
  MlirOperation subOp = mlirOperationCreate(&state);

  // Insert before the add op
  mlirBlockInsertOwnedOperationBefore(block, op, subOp);

  // Replace all uses of add result with sub result
  MlirValue addResult = mlirOperationGetResult(op, 0);
  MlirValue subResult = mlirOperationGetResult(subOp, 0);
  mlirValueReplaceAllUsesOfWith(addResult, subResult);

  // Erase the add op
  mlirOperationRemoveFromParent(op);
  mlirOperationDestroy(op);

  return MlirWalkResultAdvance;
}

static void transformRun(MlirOperation funcOp, void *userData) {
  mlirOperationWalk(funcOp, replaceAddWithSub, userData,
                    MlirWalkPostOrder);
}

int main() {
  // 1. Set up context
  MlirContext ctx = mlirContextCreate();
  cotInitContext(ctx);
  cirRegisterDialect(ctx);
  cirRegisterConstructs(ctx);

  // 2. Build module with func @main() -> i32
  MlirLocation loc = cirLocationFileLineCol(ctx, "test.ac", 1, 1);
  MlirModule module = mlirModuleCreateEmpty(loc);
  MlirBlock moduleBody = mlirModuleGetBody(module);

  // Create func @main() -> i32
  MlirType i32Type = mlirIntegerTypeGet(ctx, 32);
  MlirType funcInputTypes[] = {};
  MlirType funcResultTypes[] = {i32Type};
  MlirType funcType = mlirFunctionTypeGet(ctx, 0, funcInputTypes,
                                           1, funcResultTypes);

  MlirRegion bodyRegion = mlirRegionCreate();
  MlirBlock entryBlock = mlirBlockCreate(0, nullptr, nullptr);
  mlirRegionAppendOwnedBlock(bodyRegion, entryBlock);

  // Build the func.func op
  MlirOperationState funcState = mlirOperationStateGet(
      mlirStringRefCreateFromCString("func.func"), loc);
  mlirOperationStateAddAttributes(&funcState, 1,
      (MlirNamedAttribute[]){mlirNamedAttributeGet(
          mlirIdentifierGet(ctx,
              mlirStringRefCreateFromCString("sym_name")),
          mlirStringAttrGet(ctx,
              mlirStringRefCreateFromCString("main")))});
  mlirOperationStateAddAttributes(&funcState, 1,
      (MlirNamedAttribute[]){mlirNamedAttributeGet(
          mlirIdentifierGet(ctx,
              mlirStringRefCreateFromCString("function_type")),
          mlirTypeAttrGet(funcType))});
  mlirOperationStateAddOwnedRegions(&funcState, 1, &bodyRegion);
  MlirOperation funcOp = mlirOperationCreate(&funcState);
  mlirBlockAppendOwnedOperation(moduleBody, funcOp);

  // 3. Build ops in the entry block
  // %a = cir.constant 10 : i32
  MlirValue a = cirBuildConstantInt(entryBlock, loc, i32Type, 10);
  // %b = cir.constant 3 : i32
  MlirValue b = cirBuildConstantInt(entryBlock, loc, i32Type, 3);
  // %r = cir.add %a, %b : i32
  MlirValue r = cirBuildAdd(entryBlock, loc, a, b);

  // return %r
  MlirOperationState retState = mlirOperationStateGet(
      mlirStringRefCreateFromCString("func.return"), loc);
  mlirOperationStateAddOperands(&retState, 1, &r);
  MlirOperation retOp = mlirOperationCreate(&retState);
  mlirBlockAppendOwnedOperation(entryBlock, retOp);

  // 4. Create external pass and add to pipeline
  MlirPass pass = cotCreateFuncPass(
      "add-to-sub", "Replace cir.add with cir.sub",
      transformRun, nullptr);

  CotPipelineBuilder pipeline = cotPipelineBuilderCreate(ctx);
  cotPipelineBuilderAddPostSemaPass(pipeline, pass);

  // 5. Emit binary
  MlirLogicalResult result = cotPipelineBuilderEmitBinary(
      pipeline, module, "/tmp/cot_transform_test");

  if (mlirLogicalResultIsFailure(result)) {
    fprintf(stderr, "transform_api_test: pipeline failed\n");
    cotPipelineBuilderDestroy(pipeline);
    mlirModuleDestroy(module);
    mlirContextDestroy(ctx);
    return 1;
  }

  cotPipelineBuilderDestroy(pipeline);
  mlirModuleDestroy(module);
  mlirContextDestroy(ctx);

  // 6. Execute the binary — exit code should be 7 (10 - 3)
  int exitCode = system("/tmp/cot_transform_test");
  // On Unix, system() returns the status from waitpid; extract exit code
  exitCode = WEXITSTATUS(exitCode);

  if (exitCode == 7) {
    printf("PASS: transform_api_test (exit code = %d)\n", exitCode);
    return 0;
  }

  fprintf(stderr, "FAIL: expected exit code 7, got %d\n", exitCode);
  return 1;
}
