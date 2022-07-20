###############
# Mock config #
###############

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW 2.1 REQUIRED glfw3) # See: https://www.glfw.org/docs/3.0/build.html

target_include_directories(${PROJECT_NAME}
    PRIVATE ${OPENGL_INCLUDE_DIR}
    PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${OPENGL_LIBRARIES}
    PRIVATE ${GLFW_LIBRARIES}
    PRIVATE ${GLEW_LIBRARIES})
