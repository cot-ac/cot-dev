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

# Phase 2 gate test — requires cot-core
if(COT_HAS_CORE)
  add_executable(e2e_phase2_test e2e_phase2_test.cpp)

  target_include_directories(e2e_phase2_test PRIVATE
    "${COT_CORE_SOURCE_DIR}/include"
    "${COT_CORE_BUILD_DIR}/include")

  if(APPLE)
    target_link_options(e2e_phase2_test PRIVATE
      "-Wl,-force_load,${COT_CORE_BUILD_DIR}/lib/libCotCore.a")
  else()
    target_link_options(e2e_phase2_test PRIVATE
      "-Wl,--whole-archive" "${COT_CORE_BUILD_DIR}/lib/libCotCore.a"
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
endif()
