#include <cube/hal/opengl/window.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

using namespace cube::hal::opengl;

namespace
{
    void glfw_error_callback(int, const char * const desc)
    {
        std::cerr << "GLFW error: " << desc << "\n";
    }
}

window & window::instance(window_properties creation_properties)
{
    static window instance{creation_properties};
    return instance;
}

window::~window()
{
    if (glfw_window_)
        glfwDestroyWindow(glfw_window_);
    glfwTerminate();
}

void window::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window::update()
{
    glfwPollEvents();
    glfwSwapBuffers(glfw_window_);
}

bool window::close()
{
    return glfwWindowShouldClose(glfw_window_) || glfwGetKey(glfw_window_, GLFW_KEY_ESCAPE);
}

window::window(window_properties properties)
{
    window_init(properties);
    input_init();
}

void window::window_init(window_properties const & properties)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    glfw_window_ = glfwCreateWindow(properties.width, properties.height, properties.title, NULL, NULL);
    if (!glfw_window_)
        exit(EXIT_FAILURE);
    glfwMakeContextCurrent(glfw_window_);

    if (glewInit())
        exit(EXIT_FAILURE);
}

void window::input_init()
{
    glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GL_TRUE);
}
