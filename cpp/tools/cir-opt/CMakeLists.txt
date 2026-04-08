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

# Link core constructs via -force_load (prevents dead-stripping of
# COT_REGISTER_CONSTRUCT static ctors).
# Include directories already added by parent CMakeLists.txt.

# Helper macro: force-load a construct library from core/
macro(link_core_construct dir_name lib_name var_name)
  if(COT_HAS_${var_name})
    set(_lib "${CORE_BUILD_DIR}/${dir_name}/cpp/lib${lib_name}.a")
    if(APPLE)
      target_link_options(cir-opt PRIVATE "-Wl,-force_load,${_lib}")
    else()
      target_link_options(cir-opt PRIVATE
        "-Wl,--whole-archive" "${_lib}" "-Wl,--no-whole-archive")
    endif()
  endif()
endmacro()

link_core_construct(arith CotArith CORE)
if(COT_HAS_CORE)
  target_link_libraries(cir-opt PRIVATE
    MLIRCastInterfaces MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(memory CotMemory MEMORY)
if(COT_HAS_MEMORY)
  target_link_libraries(cir-opt PRIVATE MLIRInferTypeOpInterface)
endif()

link_core_construct(flow CotFlow FLOW)
if(COT_HAS_FLOW)
  target_link_libraries(cir-opt PRIVATE MLIRControlFlowInterfaces)
endif()

link_core_construct(structs CotStructs STRUCTS)
if(COT_HAS_STRUCTS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(arrays CotArrays ARRAYS)
if(COT_HAS_ARRAYS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(slices CotSlices SLICES)
if(COT_HAS_SLICES)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(optionals CotOptionals OPTIONALS)
if(COT_HAS_OPTIONALS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(errors CotErrors ERRORS)
if(COT_HAS_ERRORS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(enums CotEnums ENUMS)
if(COT_HAS_ENUMS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(unions CotUnions UNIONS)
if(COT_HAS_UNIONS)
  target_link_libraries(cir-opt PRIVATE
    MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(generics CotGenerics GENERICS)
if(COT_HAS_GENERICS)
  target_compile_definitions(cir-opt PRIVATE COT_HAS_GENERICS)
  target_link_libraries(cir-opt PRIVATE
    MLIRFuncDialect MLIRPass MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(traits CotTraits TRAITS)
if(COT_HAS_TRAITS)
  target_link_libraries(cir-opt PRIVATE
    MLIRFuncDialect MLIRInferTypeOpInterface MLIRSideEffectInterfaces)
endif()

link_core_construct(vwt CotVWT VWT)
if(COT_HAS_VWT)
  target_link_libraries(cir-opt PRIVATE
    MLIRSideEffectInterfaces)
endif()

link_core_construct(test CotTest TEST)
if(COT_HAS_TEST)
  target_link_libraries(cir-opt PRIVATE
    MLIRFuncDialect MLIRSideEffectInterfaces)
endif()

llvm_map_components_to_libnames(LLVM_LIBS
  core support target
  aarch64info aarch64desc aarch64codegen aarch64asmparser
  x86info x86desc x86codegen x86asmparser
)
target_link_libraries(cir-opt PRIVATE ${LLVM_LIBS})
