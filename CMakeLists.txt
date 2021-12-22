##################
# Project config #
##################

cmake_minimum_required(VERSION 3.11.0)
project(led-cube-engine VERSION 0.0.0)

# Options
set(LCE_TARGET "mock" CACHE STRING "Target LED cube graphics device.")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)

# Configure CPack
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
