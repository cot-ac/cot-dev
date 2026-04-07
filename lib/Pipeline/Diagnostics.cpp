//===- Diagnostics.cpp - Pipeline debugger implementation ------*- C++ -*-===//
//
// Reference: MLIR IRPrinterInstrumentation (mlir/Pass/PassManager.h)
//
//===----------------------------------------------------------------------===//
#include "cot/Pipeline/Diagnostics.h"

#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"

#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace cot;

PipelineDebugInstrumentation::PipelineDebugInstrumentation(
    llvm::raw_ostream &os)
    : os(os) {}

PipelineDebugInstrumentation::~PipelineDebugInstrumentation() = default;

void PipelineDebugInstrumentation::runBeforePass(Pass *pass, Operation *op) {
  os << "\n//===-------------------------------------------"
        "-------------------------===//\n";
  os << "// IR BEFORE pass: " << pass->getName() << "\n";
  os << "//===-------------------------------------------"
        "-------------------------===//\n";

  // Capture snapshot for diff on failure
  {
    llvm::raw_string_ostream snapshot(beforeSnapshot);
    op->print(snapshot, OpPrintingFlags().useLocalScope());
    snapshot.flush();
  }

  op->print(os, OpPrintingFlags().useLocalScope());
  os << "\n";
}

void PipelineDebugInstrumentation::runAfterPass(Pass *pass, Operation *op) {
  os << "\n//===-------------------------------------------"
        "-------------------------===//\n";
  os << "// IR AFTER pass: " << pass->getName() << " [SUCCESS]\n";
  os << "//===-------------------------------------------"
        "-------------------------===//\n";
  op->print(os, OpPrintingFlags().useLocalScope());
  os << "\n";
  beforeSnapshot.clear();
}

void PipelineDebugInstrumentation::runAfterPassFailed(Pass *pass,
                                                       Operation *op) {
  os << "\n//===-------------------------------------------"
        "-------------------------===//\n";
  os << "// *** PASS FAILED: " << pass->getName() << " ***\n";
  os << "//===-------------------------------------------"
        "-------------------------===//\n";
  os << "// --- IR BEFORE (snapshot) ---\n";
  os << beforeSnapshot << "\n";
  os << "// --- IR AFTER (possibly invalid) ---\n";
  op->print(os, OpPrintingFlags().useLocalScope());
  os << "\n";
  beforeSnapshot.clear();
}

std::unique_ptr<PassInstrumentation>
cot::createPipelineDebugInstrumentation(llvm::raw_ostream &os) {
  return std::make_unique<PipelineDebugInstrumentation>(os);
}
