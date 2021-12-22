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

# Core
set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ${Boost_LIBRARIES})

# OpenGL
# Todo: eventually make this optional and a build setting, e.g. build for actual hardware or for simulating the cube locally
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW 2.1 REQUIRED glfw3) # See: https://www.glfw.org/docs/3.0/build.html

string(TOUPPER "TARGET_${LCE_TARGET}" TARGET_DEFINITION)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE ${TARGET_DEFINITION})
target_include_directories(${PROJECT_NAME}
    PRIVATE ${OPENGL_INCLUDE_DIR}
    PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${OPENGL_LIBRARIES}
    PRIVATE ${GLFW_LIBRARIES}
    PRIVATE ${GLEW_LIBRARIES})

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
