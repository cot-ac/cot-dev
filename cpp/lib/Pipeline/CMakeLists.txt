add_mlir_library(COTPipeline
  Pipeline.cpp
  CIRToLLVM.cpp
  Diagnostics.cpp

  LINK_LIBS PUBLIC
  CIR
  COTConstruct
  MLIRIR
  MLIRArithDialect
  MLIRArithToLLVM
  MLIRFuncDialect
  MLIRFuncToLLVM
  MLIRLLVMDialect
  MLIRLLVMCommonConversion
  MLIRPass
  MLIRReconcileUnrealizedCasts
  MLIRTransformUtils
  MLIRBuiltinToLLVMIRTranslation
  MLIRLLVMToLLVMIRTranslation
  MLIRTargetLLVMIRExport
)
