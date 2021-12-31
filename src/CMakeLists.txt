#####################
# Executable config #
#####################

set(HAL_TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hal/${LCE_TARGET}")
if (NOT EXISTS "${HAL_TARGET_DIR}")
    message(FATAL_ERROR "Unable to find target: ${LCE_TARGET}")
endif ()
message(NOTICE "Found target: ${LCE_TARGET}")

file(GLOB_RECURSE CUBE_SOURCES "cube/*.cpp")
file(GLOB_RECURSE HAL_SOURCES "${HAL_TARGET_DIR}/*.cpp")
add_executable(${PROJECT_NAME}
    "main.cpp"
    ${CUBE_SOURCES}
    ${HAL_SOURCES})

add_subdirectory(${HAL_TARGET_DIR}) # Allow for target to configure and link in additional libraries
string(TOUPPER "TARGET_${LCE_TARGET}" TARGET_DEFINITION)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE ${TARGET_DEFINITION})

set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ${Boost_LIBRARIES})
target_compile_options(${PROJECT_NAME}
    PRIVATE -Wall -Wextra -Wconversion -pedantic -O2)
