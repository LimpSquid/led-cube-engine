# Package config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# The library sources
set(SOURCES "net/tcp_server.cpp"
            "net/tcp_client.cpp"
            "net/tcp_client_management.cpp"
            "net/routing/router_node.cpp"
            "net/routing/router_handler.cpp"
            "net/routing/tokens.cpp"
            "net/routing/routing_params.cpp"
            "net/uri.cpp"
            "http/http_server.cpp"
            "http/http_client.cpp"
            "http/http_handler.cpp"
            "http/util.cpp")

# Create library
add_library(rest STATIC ${SOURCES} ${HEADERS})
target_include_directories(rest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) # Both externally and internally visible

# Private linking and include directories
target_include_directories(rest PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(rest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(rest PRIVATE ${Boost_LIBRARIES})

# Install rules
install(TARGETS rest DESTINATION lib)