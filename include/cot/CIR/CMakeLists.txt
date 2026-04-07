set(LLVM_TARGET_DEFINITIONS CIRDialect.td)
mlir_tablegen(CIRDialect.h.inc -gen-dialect-decls)
mlir_tablegen(CIRDialect.cpp.inc -gen-dialect-defs)
add_public_tablegen_target(CIRDialectIncGen)

set(LLVM_TARGET_DEFINITIONS CIROps.td)
mlir_tablegen(CIROps.h.inc -gen-op-decls)
mlir_tablegen(CIROps.cpp.inc -gen-op-defs)
add_public_tablegen_target(CIROpsIncGen)

set(LLVM_TARGET_DEFINITIONS CIRTypes.td)
mlir_tablegen(CIRTypes.h.inc -gen-typedef-decls)
mlir_tablegen(CIRTypes.cpp.inc -gen-typedef-defs)
add_public_tablegen_target(CIRTypesIncGen)

set(LLVM_TARGET_DEFINITIONS CIRInterfaces.td)
mlir_tablegen(CIRInterfaces.h.inc -gen-type-interface-decls)
mlir_tablegen(CIRInterfaces.cpp.inc -gen-type-interface-defs)
add_public_tablegen_target(CIRInterfacesIncGen)
