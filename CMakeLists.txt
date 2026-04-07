cmake_minimum_required(VERSION 3.20)
project(COT VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

add_definitions(${LLVM_DEFINITIONS})

# Discover core/ monorepo (default constructs)
set(CORE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../core" CACHE PATH
    "Path to core constructs source")
set(CORE_BUILD_DIR "${CORE_SOURCE_DIR}/build" CACHE PATH
    "Path to core constructs build")

# Map of construct names to library names and CMake variable names
set(CORE_CONSTRUCTS
  "arith:CotArith:CORE"
  "memory:CotMemory:MEMORY"
  "flow:CotFlow:FLOW"
  "structs:CotStructs:STRUCTS"
  "arrays:CotArrays:ARRAYS"
  "slices:CotSlices:SLICES"
  "optionals:CotOptionals:OPTIONALS"
  "errors:CotErrors:ERRORS"
  "test:CotTest:TEST"
  "enums:CotEnums:ENUMS"
  "unions:CotUnions:UNIONS"
  "generics:CotGenerics:GENERICS"
  "traits:CotTraits:TRAITS"
)

foreach(entry ${CORE_CONSTRUCTS})
  string(REPLACE ":" ";" parts ${entry})
  list(GET parts 0 dir_name)
  list(GET parts 1 lib_name)
  list(GET parts 2 var_name)

  set(lib_path "${CORE_BUILD_DIR}/${dir_name}/lib/lib${lib_name}.a")
  if(EXISTS "${lib_path}")
    add_library(${lib_name} STATIC IMPORTED GLOBAL)
    set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION "${lib_path}")
    message(STATUS "Found ${dir_name}: ${lib_path}")
    set(COT_HAS_${var_name} ON)

    # Add include paths for this construct
    include_directories("${CORE_SOURCE_DIR}/${dir_name}/include")
    include_directories("${CORE_BUILD_DIR}/${dir_name}/include")
  else()
    message(STATUS "${dir_name} not found — build core/ first")
    set(COT_HAS_${var_name} OFF)
  endif()
endforeach()

add_subdirectory(include/cot/CIR)
add_subdirectory(lib/CIR)
add_subdirectory(lib/Pipeline)
add_subdirectory(lib/Construct)
add_subdirectory(c-api)
add_subdirectory(runtime)
add_subdirectory(tools)
add_subdirectory(test)
