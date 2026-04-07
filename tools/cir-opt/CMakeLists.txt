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

llvm_map_components_to_libnames(LLVM_LIBS
  core support target
  aarch64info aarch64desc aarch64codegen aarch64asmparser
  x86info x86desc x86codegen x86asmparser
)
target_link_libraries(cir-opt PRIVATE ${LLVM_LIBS})
