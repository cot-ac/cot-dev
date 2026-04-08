//===- Construct.cpp - Construct registry implementation ------*- C++ -*-===//
#include "cot/Construct/Construct.h"

#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

using namespace cot;

llvm::SmallVector<std::unique_ptr<Construct>> &cot::getConstructRegistry() {
  static llvm::SmallVector<std::unique_ptr<Construct>> registry;
  return registry;
}

void cot::validateConstructDependencies() {
  auto &registry = getConstructRegistry();

  // Build set of available construct names
  llvm::StringSet<> available;
  for (auto &c : registry)
    available.insert(c->getName());

  // Validate each construct's dependencies
  for (auto &c : registry) {
    for (auto dep : c->getRequiredConstructs()) {
      if (!available.contains(dep)) {
        llvm::errs() << "cot: construct '" << c->getName()
                     << "' requires '" << dep
                     << "' which is not loaded\n";
        llvm_unreachable("missing construct dependency");
      }
    }
  }
}
