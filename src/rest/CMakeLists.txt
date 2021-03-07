# Package config
set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

# The library sources
set(SOURCES "tcpserver.cpp"
            "tcpclient.cpp"
            "tcpclientmanager.cpp"
            "requestparser.cpp"
            "http/httpparser.cpp"
            "http/httpserver.cpp"
            "http/httpclient.cpp")

# Create library
add_library(rest STATIC ${SOURCES} ${HEADERS})
target_include_directories(rest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) # Both externally and internally visible

# Private linking and include directories
target_include_directories(rest PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(rest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(rest PRIVATE ${Boost_LIBRARIES})

# Install rules
install(TARGETS rest DESTINATION lib)