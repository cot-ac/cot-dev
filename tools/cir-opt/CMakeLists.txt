add_llvm_executable(cir-opt
  CIROpt.cpp
)

target_link_libraries(cir-opt PRIVATE
  CIR
  COTPipeline
  COTConstruct
  MLIRArithDialect
  MLIRFuncDialect
  MLIRLLVMDialect
  MLIRIR
  MLIROptLib
  MLIRPass
)

# Link cot-core construct when available.
# -force_load prevents dead-stripping of COT_REGISTER_CONSTRUCT static ctors.
if(COT_HAS_CORE)
  target_include_directories(cir-opt PRIVATE
    "${COT_CORE_SOURCE_DIR}/include"
    "${COT_CORE_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_CORE_BUILD_DIR}/lib/libCotCore.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_CORE_BUILD_DIR}/lib/libCotCore.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRCastInterfaces
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-memory construct when available.
if(COT_HAS_MEMORY)
  target_include_directories(cir-opt PRIVATE
    "${COT_MEMORY_SOURCE_DIR}/include"
    "${COT_MEMORY_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_MEMORY_BUILD_DIR}/lib/libCotMemory.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_MEMORY_BUILD_DIR}/lib/libCotMemory.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
  )
endif()

# Link cot-flow construct when available.
if(COT_HAS_FLOW)
  target_include_directories(cir-opt PRIVATE
    "${COT_FLOW_SOURCE_DIR}/include"
    "${COT_FLOW_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_FLOW_BUILD_DIR}/lib/libCotFlow.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_FLOW_BUILD_DIR}/lib/libCotFlow.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRControlFlowInterfaces
  )
endif()

# Link cot-structs construct when available.
if(COT_HAS_STRUCTS)
  target_include_directories(cir-opt PRIVATE
    "${COT_STRUCTS_SOURCE_DIR}/include"
    "${COT_STRUCTS_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_STRUCTS_BUILD_DIR}/lib/libCotStructs.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_STRUCTS_BUILD_DIR}/lib/libCotStructs.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-arrays construct when available.
if(COT_HAS_ARRAYS)
  target_include_directories(cir-opt PRIVATE
    "${COT_ARRAYS_SOURCE_DIR}/include"
    "${COT_ARRAYS_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_ARRAYS_BUILD_DIR}/lib/libCotArrays.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_ARRAYS_BUILD_DIR}/lib/libCotArrays.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-slices construct when available.
if(COT_HAS_SLICES)
  target_include_directories(cir-opt PRIVATE
    "${COT_SLICES_SOURCE_DIR}/include"
    "${COT_SLICES_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_SLICES_BUILD_DIR}/lib/libCotSlices.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_SLICES_BUILD_DIR}/lib/libCotSlices.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-optionals construct when available.
if(COT_HAS_OPTIONALS)
  target_include_directories(cir-opt PRIVATE
    "${COT_OPTIONALS_SOURCE_DIR}/include"
    "${COT_OPTIONALS_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_OPTIONALS_BUILD_DIR}/lib/libCotOptionals.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_OPTIONALS_BUILD_DIR}/lib/libCotOptionals.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-errors construct when available.
if(COT_HAS_ERRORS)
  target_include_directories(cir-opt PRIVATE
    "${COT_ERRORS_SOURCE_DIR}/include"
    "${COT_ERRORS_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_ERRORS_BUILD_DIR}/lib/libCotErrors.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_ERRORS_BUILD_DIR}/lib/libCotErrors.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface
    MLIRSideEffectInterfaces
  )
endif()

# Link cot-test construct when available.
if(COT_HAS_TEST)
  target_include_directories(cir-opt PRIVATE
    "${COT_TEST_SOURCE_DIR}/include"
    "${COT_TEST_BUILD_DIR}/include")
  if(APPLE)
    target_link_options(cir-opt PRIVATE
      "-Wl,-force_load,${COT_TEST_BUILD_DIR}/lib/libCotTest.a")
  else()
    target_link_options(cir-opt PRIVATE
      "-Wl,--whole-archive" "${COT_TEST_BUILD_DIR}/lib/libCotTest.a"
      "-Wl,--no-whole-archive")
  endif()
  target_link_libraries(cir-opt PRIVATE
    MLIRFuncDialect
    MLIRSideEffectInterfaces
  )
endif()

llvm_map_components_to_libnames(LLVM_LIBS
  core support target
  aarch64info aarch64desc aarch64codegen aarch64asmparser
  x86info x86desc x86codegen x86asmparser
)
target_link_libraries(cir-opt PRIVATE ${LLVM_LIBS})
