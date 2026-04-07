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

# Include construct headers for C API builder functions
if(COT_HAS_CORE)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_CORE)
  target_include_directories(obj.COTCApi PRIVATE
    "${COT_CORE_SOURCE_DIR}/include"
    "${COT_CORE_BUILD_DIR}/include")
endif()

if(COT_HAS_MEMORY)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_MEMORY)
  target_include_directories(obj.COTCApi PRIVATE
    "${COT_MEMORY_SOURCE_DIR}/include"
    "${COT_MEMORY_BUILD_DIR}/include")
endif()

if(COT_HAS_FLOW)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_FLOW)
  target_include_directories(obj.COTCApi PRIVATE
    "${COT_FLOW_SOURCE_DIR}/include"
    "${COT_FLOW_BUILD_DIR}/include")
endif()
