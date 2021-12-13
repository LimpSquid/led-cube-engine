#include <cube/hal/mock/window.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

using namespace cube::hal::mock;

namespace
{

constexpr float translation_step = 0.02;
constexpr float rotation_step = 1.25;

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
    gluPerspective(60, static_cast<float>(width) / height, 0.1, 30); // Todo: magic variables

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glTranslatef(-camera_.translation.x, -camera_.translation.y, -camera_.translation.z);
    // Todo: should be possible to do this with one function right?
    glRotatef(camera_.rotation.x, 1, 0, 0);
    glRotatef(camera_.rotation.y, 0, 1, 0);
    glRotatef(camera_.rotation.z, 0, 0, 1);

    glfwPollEvents();
    process_inputs();
    glfwSwapBuffers(glfw_window_);
}

bool window::close()
{
    return glfwWindowShouldClose(glfw_window_);
}

window::window(window_properties properties)
{
    init_window(properties);
    init_inputs();
}

void window::init_window(window_properties const & properties)
{
    // Init glfw
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

    glfwSetWindowUserPointer(glfw_window_, this);

    if (glewInit())
        exit(EXIT_FAILURE);

    // Init camera
    camera_.translation = glm::vec3(0, 0, (properties.width * 1.5f) / properties.height);
    camera_.rotation = glm::vec3(120, 0, 22.5);
}

void window::init_inputs()
{
    glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(glfw_window_, glfw_key_callback);
}

void window::glfw_key_callback(GLFWwindow * const glfw_window, int key, int scancode, int action, int modifiers)
{
	window & self = *reinterpret_cast<window *>(glfwGetWindowUserPointer(glfw_window));
    if (action == GLFW_PRESS)
        self.process_key_press(key, scancode, modifiers);
}

void window::process_key_press(int key, int /* scancode */, int modifiers)
{
    switch(key) {
        case GLFW_KEY_ESCAPE:   glfwSetWindowShouldClose(glfw_window_, 1);  break;
        default:;
    }
}

void window::process_inputs()
{
    auto update_camera = [this](int key, auto rotation_handler, auto translation_handler) {
        if (glfwGetKey(glfw_window_, key) == GLFW_PRESS) {
            if (glfwGetKey(glfw_window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                rotation_handler();
            else
                translation_handler();
        }
    };

    update_camera(GLFW_KEY_UP,
        [this](){ camera_.rotation.x -= rotation_step; },
        [this](){ camera_.translation.y -= translation_step; });
    update_camera(GLFW_KEY_DOWN,
        [this](){ camera_.rotation.x += rotation_step; },
        [this](){ camera_.translation.y += translation_step; });
    update_camera(GLFW_KEY_RIGHT,
        [this](){ camera_.rotation.y -= rotation_step; },
        [this](){ camera_.translation.x -= translation_step; });
    update_camera(GLFW_KEY_LEFT,
        [this](){ camera_.rotation.y += rotation_step; },
        [this](){ camera_.translation.x += translation_step; });
    update_camera(GLFW_KEY_PAGE_UP,
        [this](){ camera_.rotation.z -= rotation_step; },
        [this](){ camera_.translation.z -= translation_step; });
    update_camera(GLFW_KEY_PAGE_DOWN,
        [this](){ camera_.rotation.z += rotation_step; },
        [this](){ camera_.translation.z += translation_step; });
}
