##################
# Project config #
##################

cmake_minimum_required(VERSION 3.13.0)
project(led-cube-engine VERSION 0.0.0)

# Options
set(LCE_TARGET "mock" CACHE STRING "Target LED cube engine graphics device.")
set(LCE_EVAL_EXPRESSIONS OFF CACHE BOOL "Enable string expressions for JSON number fields.")
set(LCE_BACKTRACE_SYMBOLS ON CACHE BOOL "Enable human readable backtrace symbols.")
set(LCE_HTTP_API ON CACHE BOOL "Enable HTTP api.")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)

# Configure CPack
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
