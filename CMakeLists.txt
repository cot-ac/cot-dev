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

# Discover cot-core construct (sibling repo)
set(COT_CORE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-core" CACHE PATH
    "Path to cot-core source")
set(COT_CORE_BUILD_DIR "${COT_CORE_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-core build")

if(EXISTS "${COT_CORE_BUILD_DIR}/lib/libCotCore.a")
  add_library(CotCore STATIC IMPORTED GLOBAL)
  set_target_properties(CotCore PROPERTIES
    IMPORTED_LOCATION "${COT_CORE_BUILD_DIR}/lib/libCotCore.a")
  message(STATUS "Found cot-core: ${COT_CORE_BUILD_DIR}/lib/libCotCore.a")
  set(COT_HAS_CORE ON)
else()
  message(STATUS "cot-core not found — build cot-core first for Phase 2 tests")
  set(COT_HAS_CORE OFF)
endif()

# Discover cot-memory construct (sibling repo)
set(COT_MEMORY_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-memory" CACHE PATH
    "Path to cot-memory source")
set(COT_MEMORY_BUILD_DIR "${COT_MEMORY_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-memory build")

if(EXISTS "${COT_MEMORY_BUILD_DIR}/lib/libCotMemory.a")
  add_library(CotMemory STATIC IMPORTED GLOBAL)
  set_target_properties(CotMemory PROPERTIES
    IMPORTED_LOCATION "${COT_MEMORY_BUILD_DIR}/lib/libCotMemory.a")
  message(STATUS "Found cot-memory: ${COT_MEMORY_BUILD_DIR}/lib/libCotMemory.a")
  set(COT_HAS_MEMORY ON)
else()
  message(STATUS "cot-memory not found — build cot-memory first")
  set(COT_HAS_MEMORY OFF)
endif()

# Discover cot-flow construct (sibling repo)
set(COT_FLOW_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-flow" CACHE PATH
    "Path to cot-flow source")
set(COT_FLOW_BUILD_DIR "${COT_FLOW_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-flow build")

if(EXISTS "${COT_FLOW_BUILD_DIR}/lib/libCotFlow.a")
  add_library(CotFlow STATIC IMPORTED GLOBAL)
  set_target_properties(CotFlow PROPERTIES
    IMPORTED_LOCATION "${COT_FLOW_BUILD_DIR}/lib/libCotFlow.a")
  message(STATUS "Found cot-flow: ${COT_FLOW_BUILD_DIR}/lib/libCotFlow.a")
  set(COT_HAS_FLOW ON)
else()
  message(STATUS "cot-flow not found — build cot-flow first")
  set(COT_HAS_FLOW OFF)
endif()

# Discover cot-structs construct (sibling repo)
set(COT_STRUCTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-structs" CACHE PATH
    "Path to cot-structs source")
set(COT_STRUCTS_BUILD_DIR "${COT_STRUCTS_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-structs build")

if(EXISTS "${COT_STRUCTS_BUILD_DIR}/lib/libCotStructs.a")
  add_library(CotStructs STATIC IMPORTED GLOBAL)
  set_target_properties(CotStructs PROPERTIES
    IMPORTED_LOCATION "${COT_STRUCTS_BUILD_DIR}/lib/libCotStructs.a")
  message(STATUS "Found cot-structs: ${COT_STRUCTS_BUILD_DIR}/lib/libCotStructs.a")
  set(COT_HAS_STRUCTS ON)
else()
  message(STATUS "cot-structs not found — build cot-structs first")
  set(COT_HAS_STRUCTS OFF)
endif()

# Discover cot-arrays construct (sibling repo)
set(COT_ARRAYS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-arrays" CACHE PATH
    "Path to cot-arrays source")
set(COT_ARRAYS_BUILD_DIR "${COT_ARRAYS_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-arrays build")

if(EXISTS "${COT_ARRAYS_BUILD_DIR}/lib/libCotArrays.a")
  add_library(CotArrays STATIC IMPORTED GLOBAL)
  set_target_properties(CotArrays PROPERTIES
    IMPORTED_LOCATION "${COT_ARRAYS_BUILD_DIR}/lib/libCotArrays.a")
  message(STATUS "Found cot-arrays: ${COT_ARRAYS_BUILD_DIR}/lib/libCotArrays.a")
  set(COT_HAS_ARRAYS ON)
else()
  message(STATUS "cot-arrays not found — build cot-arrays first")
  set(COT_HAS_ARRAYS OFF)
endif()

# Discover cot-slices construct (sibling repo)
set(COT_SLICES_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-slices" CACHE PATH
    "Path to cot-slices source")
set(COT_SLICES_BUILD_DIR "${COT_SLICES_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-slices build")

if(EXISTS "${COT_SLICES_BUILD_DIR}/lib/libCotSlices.a")
  add_library(CotSlices STATIC IMPORTED GLOBAL)
  set_target_properties(CotSlices PROPERTIES
    IMPORTED_LOCATION "${COT_SLICES_BUILD_DIR}/lib/libCotSlices.a")
  message(STATUS "Found cot-slices: ${COT_SLICES_BUILD_DIR}/lib/libCotSlices.a")
  set(COT_HAS_SLICES ON)
else()
  message(STATUS "cot-slices not found — build cot-slices first")
  set(COT_HAS_SLICES OFF)
endif()

# Discover cot-optionals construct (sibling repo)
set(COT_OPTIONALS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-optionals" CACHE PATH
    "Path to cot-optionals source")
set(COT_OPTIONALS_BUILD_DIR "${COT_OPTIONALS_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-optionals build")

if(EXISTS "${COT_OPTIONALS_BUILD_DIR}/lib/libCotOptionals.a")
  add_library(CotOptionals STATIC IMPORTED GLOBAL)
  set_target_properties(CotOptionals PROPERTIES
    IMPORTED_LOCATION "${COT_OPTIONALS_BUILD_DIR}/lib/libCotOptionals.a")
  message(STATUS "Found cot-optionals: ${COT_OPTIONALS_BUILD_DIR}/lib/libCotOptionals.a")
  set(COT_HAS_OPTIONALS ON)
else()
  message(STATUS "cot-optionals not found — build cot-optionals first")
  set(COT_HAS_OPTIONALS OFF)
endif()

# Discover cot-errors construct (sibling repo)
set(COT_ERRORS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../cot-errors" CACHE PATH
    "Path to cot-errors source")
set(COT_ERRORS_BUILD_DIR "${COT_ERRORS_SOURCE_DIR}/build" CACHE PATH
    "Path to cot-errors build")

if(EXISTS "${COT_ERRORS_BUILD_DIR}/lib/libCotErrors.a")
  add_library(CotErrors STATIC IMPORTED GLOBAL)
  set_target_properties(CotErrors PROPERTIES
    IMPORTED_LOCATION "${COT_ERRORS_BUILD_DIR}/lib/libCotErrors.a")
  message(STATUS "Found cot-errors: ${COT_ERRORS_BUILD_DIR}/lib/libCotErrors.a")
  set(COT_HAS_ERRORS ON)
else()
  message(STATUS "cot-errors not found — build cot-errors first")
  set(COT_HAS_ERRORS OFF)
endif()

add_subdirectory(include/cot/CIR)
add_subdirectory(lib/CIR)
add_subdirectory(lib/Pipeline)
add_subdirectory(lib/Construct)
add_subdirectory(c-api)
add_subdirectory(runtime)
add_subdirectory(tools)
add_subdirectory(test)
