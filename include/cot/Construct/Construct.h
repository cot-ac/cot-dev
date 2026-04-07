//===- Construct.h - Construct base class + registration ------*- C++ -*-===//
#ifndef COT_CONSTRUCT_CONSTRUCT_H
#define COT_CONSTRUCT_CONSTRUCT_H

#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/ExtensibleDialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"

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

  /// Add this construct's CIR->CIR transformer passes to the pipeline.
  /// Called by PipelineBuilder during pipeline composition.
  ///
  /// The PassManager already has the correct nesting (module -> func).
  /// Add passes at the appropriate extension point:
  ///   preSemaPM  — before type checking (e.g., witness thunk generation)
  ///   postSemaPM — after type checking (e.g., ARC optimization)
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
/// This creates a global constructor that registers the construct.
/// For static linking: the construct is registered at program startup.
/// For dynamic loading (dlopen): registered when the plugin loads.
#define COT_REGISTER_CONSTRUCT(ConstructClass) \
  static ConstructRegistration<ConstructClass> \
      _cot_construct_##ConstructClass##_registration;

#endif // COT_CONSTRUCT_CONSTRUCT_H
