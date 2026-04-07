add_library(cir_arc STATIC cir_arc.c)
target_include_directories(cir_arc PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
