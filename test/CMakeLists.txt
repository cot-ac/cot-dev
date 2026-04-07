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
endif()
