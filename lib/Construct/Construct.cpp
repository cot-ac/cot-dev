//===- Construct.cpp - Construct registry implementation ------*- C++ -*-===//
#include "cot/Construct/Construct.h"

using namespace cot;

llvm::SmallVector<std::unique_ptr<Construct>> &cot::getConstructRegistry() {
  static llvm::SmallVector<std::unique_ptr<Construct>> registry;
  return registry;
}
