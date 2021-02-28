# Package config
set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

# Configure library
set(HEADERS "server.h"
            "client.h")
set(SOURCES "server.cpp"
            "client.cpp")

add_library(rest STATIC ${SOURCES})
target_include_directories(rest PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(rest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(rest PRIVATE ${Boost_LIBRARIES})

# Install rules
install(TARGETS rest DESTINATION lib)