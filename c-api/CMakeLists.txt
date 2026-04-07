add_mlir_library(COTCApi
  CIRCApi.cpp
  COTCApi.cpp

  LINK_LIBS PUBLIC
  CIR
  COTPipeline
  MLIRIR
  MLIRCAPIIR
  MLIRCAPITransforms
  MLIRFuncDialect
  MLIRLLVMDialect
)
