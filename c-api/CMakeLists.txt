add_mlir_library(COTCApi
  CIRCApi.cpp
  COTCApi.cpp

  LINK_LIBS PUBLIC
  CIR
  COTConstruct
  COTPipeline
  MLIRIR
  MLIRCAPIIR
  MLIRCAPITransforms
  MLIRFuncDialect
  MLIRLLVMDialect
)

# Include directories already set by parent CMakeLists.txt foreach loop.
# Just add compile definitions for conditional compilation.
if(COT_HAS_CORE)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_CORE)
endif()

if(COT_HAS_MEMORY)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_MEMORY)
endif()

if(COT_HAS_FLOW)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_FLOW)
endif()
