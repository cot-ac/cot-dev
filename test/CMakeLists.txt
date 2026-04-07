add_executable(framework_test framework_test.cpp)

target_link_libraries(framework_test PRIVATE
  COTPipeline
  COTConstruct
  COTCApi
  CIR
  cir_arc
  MLIRArithDialect
  MLIRFuncDialect
  MLIRLLVMDialect
  MLIRIR
  MLIRPass
)

# Phase 2 gate test — requires arith (was cot-core)
if(COT_HAS_CORE)
  add_executable(e2e_phase2_test e2e_phase2_test.cpp)

  # Include dirs already set by parent CMakeLists.txt foreach loop

  if(APPLE)
    target_link_options(e2e_phase2_test PRIVATE
      "-Wl,-force_load,${CORE_BUILD_DIR}/arith/lib/libCotArith.a")
  else()
    target_link_options(e2e_phase2_test PRIVATE
      "-Wl,--whole-archive" "${CORE_BUILD_DIR}/arith/lib/libCotArith.a"
      "-Wl,--no-whole-archive")
  endif()

  target_link_libraries(e2e_phase2_test PRIVATE
    COTPipeline
    COTConstruct
    CIR
    cir_arc
    MLIRArithDialect
    MLIRFuncDialect
    MLIRLLVMDialect
    MLIRIR
    MLIRPass
    MLIRCastInterfaces
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )

  # Link memory if available
  if(COT_HAS_MEMORY)
    target_compile_definitions(e2e_phase2_test PRIVATE COT_HAS_MEMORY)
    if(APPLE)
      target_link_options(e2e_phase2_test PRIVATE
        "-Wl,-force_load,${CORE_BUILD_DIR}/memory/lib/libCotMemory.a")
    else()
      target_link_options(e2e_phase2_test PRIVATE
        "-Wl,--whole-archive" "${CORE_BUILD_DIR}/memory/lib/libCotMemory.a"
        "-Wl,--no-whole-archive")
    endif()
  endif()

  # Transform C API test — validates external pass creation via C API
  add_executable(transform_api_test transform_api_test.cpp)

  # Helper macro for force-loading a construct .a
  macro(link_core_construct_test target construct_name lib_name)
    if(EXISTS "${CORE_BUILD_DIR}/${construct_name}/lib/lib${lib_name}.a")
      if(APPLE)
        target_link_options(${target} PRIVATE
          "-Wl,-force_load,${CORE_BUILD_DIR}/${construct_name}/lib/lib${lib_name}.a")
      else()
        target_link_options(${target} PRIVATE
          "-Wl,--whole-archive"
          "${CORE_BUILD_DIR}/${construct_name}/lib/lib${lib_name}.a"
          "-Wl,--no-whole-archive")
      endif()
    endif()
  endmacro()

  link_core_construct_test(transform_api_test arith CotArith)
  link_core_construct_test(transform_api_test memory CotMemory)
  link_core_construct_test(transform_api_test flow CotFlow)
  link_core_construct_test(transform_api_test structs CotStructs)
  link_core_construct_test(transform_api_test arrays CotArrays)
  link_core_construct_test(transform_api_test slices CotSlices)
  link_core_construct_test(transform_api_test optionals CotOptionals)
  link_core_construct_test(transform_api_test errors CotErrors)
  link_core_construct_test(transform_api_test test CotTest)
  link_core_construct_test(transform_api_test enums CotEnums)
  link_core_construct_test(transform_api_test unions CotUnions)
  link_core_construct_test(transform_api_test generics CotGenerics)
  link_core_construct_test(transform_api_test traits CotTraits)
  link_core_construct_test(transform_api_test vwt CotVWT)

  target_link_libraries(transform_api_test PRIVATE
    COTPipeline
    COTConstruct
    COTCApi
    CIR
    cir_arc
    MLIRArithDialect
    MLIRFuncDialect
    MLIRLLVMDialect
    MLIRIR
    MLIRPass
    MLIRCAPIIR
    MLIRCAPITransforms
    MLIRCastInterfaces
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()
