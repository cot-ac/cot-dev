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

if(COT_HAS_STRUCTS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_STRUCTS)
endif()

if(COT_HAS_ARRAYS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_ARRAYS)
endif()

if(COT_HAS_SLICES)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_SLICES)
endif()

if(COT_HAS_OPTIONALS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_OPTIONALS)
endif()

if(COT_HAS_ERRORS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_ERRORS)
endif()

if(COT_HAS_TEST)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_TEST)
endif()

if(COT_HAS_ENUMS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_ENUMS)
endif()

if(COT_HAS_UNIONS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_UNIONS)
endif()

if(COT_HAS_GENERICS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_GENERICS)
endif()

if(COT_HAS_TRAITS)
  target_compile_definitions(obj.COTCApi PRIVATE COT_HAS_TRAITS)
endif()
