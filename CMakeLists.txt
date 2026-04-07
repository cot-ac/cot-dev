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

add_subdirectory(include/cot/CIR)
add_subdirectory(lib/CIR)
add_subdirectory(lib/Pipeline)
add_subdirectory(lib/Construct)
add_subdirectory(c-api)
add_subdirectory(runtime)
add_subdirectory(tools)
add_subdirectory(test)
