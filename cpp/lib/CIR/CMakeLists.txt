add_mlir_library(CIR
  CIRDialect.cpp

  DEPENDS
  CIRDialectIncGen
  CIROpsIncGen
  CIRTypesIncGen
  CIRInterfacesIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRFuncDialect
  MLIRLLVMDialect
  MLIRDLTIDialect
  MLIRInferTypeOpInterface
  MLIRCastInterfaces
  MLIRControlFlowInterfaces
  MLIRSideEffectInterfaces
)
