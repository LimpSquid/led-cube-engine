#include <cube/hal/mock/window.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

using namespace cube::hal::mock;

namespace
{

constexpr double camera_navigation_step = 0.02;

void glfw_error_callback(int, const char * const desc)
{
    std::cerr << "glfw error: " << desc << "\n";
}

} // end of namespace

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
    int width;
    int height;
    glfwGetFramebufferSize(glfw_window_, &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, static_cast<float>(width) / height, 0.1, 30);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glTranslatef(-camera_.x, -camera_.y, -camera_.z);

    glfwPollEvents();
    process_inputs();
    glfwSwapBuffers(glfw_window_);
}

bool window::close()
{
    return glfwWindowShouldClose(glfw_window_);
}

window::window(window_properties properties) :
    camera_(0, 0, (properties.width * 1.5f) / properties.height)
{
    init_window(properties);
    init_inputs();
}

void window::init_window(window_properties const & properties)
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

void window::init_inputs()
{
    glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GL_TRUE);
}

void window::process_inputs()
{
    auto process_input = [this](int key, auto h) {
        if (glfwGetKey(glfw_window_, key) == GLFW_PRESS)
            h();
    };

    process_input(GLFW_KEY_UP,          [this](){ camera_.y -= camera_navigation_step; });
    process_input(GLFW_KEY_DOWN,        [this](){ camera_.y += camera_navigation_step; });
    process_input(GLFW_KEY_RIGHT,       [this](){ camera_.x -= camera_navigation_step; });
    process_input(GLFW_KEY_LEFT,        [this](){ camera_.x += camera_navigation_step; });
    process_input(GLFW_KEY_PAGE_UP,     [this](){ camera_.z -= camera_navigation_step; });
    process_input(GLFW_KEY_PAGE_DOWN,   [this](){ camera_.z += camera_navigation_step; });
    process_input(GLFW_KEY_ESCAPE,      [this](){ glfwSetWindowShouldClose(glfw_window_, 1); });
}
