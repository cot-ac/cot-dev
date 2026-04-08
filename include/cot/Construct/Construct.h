//===- Construct.h - Construct base class + registration ------*- C++ -*-===//
#ifndef COT_CONSTRUCT_CONSTRUCT_H
#define COT_CONSTRUCT_CONSTRUCT_H

#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/ExtensibleDialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"

namespace cot { class CIRSema; }

namespace cot {

/// Base class for all COT constructs. Each construct repo implements this.
///
/// A construct provides:
/// 1. Types and ops (registered at initialization)
/// 2. Transformer passes (added to the pipeline)
/// 3. Lowering patterns (CIR -> LLVM)
/// 4. Type conversions (CIR types -> LLVM types)
class Construct {
public:
  virtual ~Construct() = default;

  /// Human-readable name for logging.
  virtual llvm::StringRef getName() const = 0;

  /// Return the names of constructs this one requires to be loaded.
  /// The framework validates at initialization that all dependencies
  /// are present. Override to declare dependencies on other constructs.
  virtual llvm::SmallVector<llvm::StringRef> getRequiredConstructs() const {
    return {};
  }

  /// Register this construct's static ops and types with the context.
  /// Called once at framework initialization.
  virtual void registerOpsAndTypes(mlir::MLIRContext &ctx) {}

  /// Register any dynamic ops with the extensible dialect.
  /// Called once at framework initialization, after static registration.
  virtual void registerDynamicOps(mlir::ExtensibleDialect &dialect) {}

  /// Register steps for the CIRSema single-walk pass.
  /// Steps run in fixed position order: Comptime → Generics → Types → Ownership.
  /// Use this for per-op sequential transforms.
  /// Use addTransformers() for separate passes needing iterative analysis.
  virtual void registerSemaSteps(CIRSema &sema) {}

  /// Add this construct's separate CIR->CIR transformer passes.
  /// Called by PipelineBuilder during pipeline composition.
  /// Use for post-sema passes that need whole-function iterative analysis
  /// (ARC optimization, devirtualization, concurrency lowering).
  /// For per-op sequential transforms, use registerSemaSteps() instead.
  virtual void addTransformers(mlir::PassManager &preSemaPM,
                               mlir::PassManager &postSemaPM) {}

  /// Add this construct's CIR->LLVM lowering patterns.
  /// Called by the CIR->LLVM lowering pass.
  virtual void populateLoweringPatterns(
      mlir::RewritePatternSet &patterns,
      mlir::TypeConverter &typeConverter) {}

  /// Add this construct's type conversions (CIR types -> LLVM types).
  /// Called once during CIR->LLVM pass setup.
  virtual void addTypeConversions(mlir::TypeConverter &typeConverter) {}

  /// Try to parse a CIR type with the given keyword (mnemonic).
  /// Return empty OptionalParseResult if this construct doesn't handle it.
  virtual mlir::OptionalParseResult parseType(
      llvm::StringRef keyword, mlir::DialectAsmParser &parser,
      mlir::Type &result) const {
    return {};
  }

  /// Try to print a CIR type. Return failure if not handled.
  virtual mlir::LogicalResult printType(
      mlir::Type type, mlir::DialectAsmPrinter &printer) const {
    return mlir::failure();
  }
};

/// Global construct registry — populated at static init.
llvm::SmallVector<std::unique_ptr<Construct>> &getConstructRegistry();

/// Validate that all construct dependencies are satisfied.
/// Call after all constructs have been registered (linked).
/// Terminates with an error if a required construct is missing.
void validateConstructDependencies();

} // namespace cot

/// Registration helper. Instantiated by COT_REGISTER_CONSTRUCT.
template <typename T>
struct ConstructRegistration {
  ConstructRegistration() {
    cot::getConstructRegistry().push_back(std::make_unique<T>());
  }
};

/// Register a construct with the framework. Place in a .cpp file:
///
///   COT_REGISTER_CONSTRUCT(CoreConstruct)
///
/// This creates a global constructor that registers the construct,
/// plus a C-linkage anchor function (_cot_anchor_<Name>) that can be
/// referenced from non-C++ build systems (Zig, Go, Swift) to prevent
/// the linker from dead-stripping this translation unit.
#define COT_REGISTER_CONSTRUCT(ConstructClass) \
  static ConstructRegistration<ConstructClass> \
      _cot_construct_##ConstructClass##_registration; \
  extern "C" void _cot_anchor_##ConstructClass() {}

#endif // COT_CONSTRUCT_CONSTRUCT_H
