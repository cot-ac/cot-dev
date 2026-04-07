add_mlir_library(COTConstruct
  Construct.cpp

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRTransformUtils
)
