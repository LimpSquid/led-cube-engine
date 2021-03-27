# Add subdirs
add_subdirectory(rest)
add_subdirectory(cube)

# Build executable
set(SOURCES "main.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# Link our executable
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} rest)
target_link_libraries(${PROJECT_NAME} cube)