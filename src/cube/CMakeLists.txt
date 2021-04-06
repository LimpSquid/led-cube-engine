# Package config
set(${THREADS_PREFER_PTHREAD_FLAG} ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# The library sources
set(SOURCES "core/engine.cpp"
            "core/painter.cpp"
            "core/graphics_device.cpp"
            "core/animation.cpp"
            "hal/voxel_display.cpp")

# Create library
add_library(cube STATIC ${SOURCES} ${HEADERS})
target_include_directories(cube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) # Both externally and internally visible

# Private linking and include directories
target_include_directories(cube PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(cube PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cube PRIVATE ${Boost_LIBRARIES})

# Install rules
install(TARGETS cube DESTINATION lib)