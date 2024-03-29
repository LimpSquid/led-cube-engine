#####################
# Executable config #
#####################

set(DRIVER_TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/driver/${LCE_TARGET}")
if (NOT EXISTS "${DRIVER_TARGET_DIR}")
    message(FATAL_ERROR "Unable to find target driver: ${LCE_TARGET}")
endif ()
message(NOTICE "Found target driver: ${LCE_TARGET}")

file(GLOB_RECURSE CUBE_SOURCES "cube/*.cpp")
file(GLOB_RECURSE DRIVER_SOURCES "${DRIVER_TARGET_DIR}/*.cpp")

if (NOT LCE_HTTP_API)
    # Exclude sources which contain the keyword 'http' in its filepath
    list(FILTER CUBE_SOURCES EXCLUDE REGEX "http.*\.cpp$")
endif ()

add_executable(${PROJECT_NAME}
    "main.cpp"
    ${CUBE_SOURCES}
    ${DRIVER_SOURCES})

add_subdirectory(${DRIVER_TARGET_DIR}) # Allow for target to configure and link in additional libraries
string(TOUPPER "TARGET_${LCE_TARGET}" TARGET_DEFINITION)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE ${TARGET_DEFINITION})

if (LCE_EVAL_EXPRESSIONS)
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE EVAL_EXPRESSION)
endif ()

# To list all boost components:
# set(Boost_DEBUG ON)
# find_package(Boost COMPONENTS ALL)

set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE stdc++fs) # Apparently need for some systems
target_compile_options(${PROJECT_NAME}
    PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wstrict-aliasing -O3)

if (LCE_BACKTRACE_SYMBOLS)
    target_compile_options(${PROJECT_NAME}
        PRIVATE -g)
    target_link_options(${PROJECT_NAME}
        PRIVATE -rdynamic)
endif ()
