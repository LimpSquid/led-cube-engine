#include <cube/hal/mock/window.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace
{

constexpr double translation_step = 0.04;
constexpr double mouse_drag_sensitivity = 0.1;

// All degrees angles down below
constexpr double x_axis_min = -160;
constexpr double x_axis_max = -20;
constexpr double z_axis_min = -360;
constexpr double z_axis_max = 360;
constexpr glm::dvec3 x_axis_view = {-90, 0, 0};
constexpr glm::dvec3 y_axis_view = {-90, 0, 90};
constexpr glm::dvec3 z_axis_view = {0, 0, 0};
constexpr glm::dvec3 default_view = x_axis_view + glm::dvec3(25, 0, -35);

void glfw_error_callback(int, const char * const desc)
{
    std::cerr << "glfw error: " << desc << "\n";
}

} // End of namespace


namespace cube::hal::mock
{

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
    draw_triad();
    compute_projection();

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
    camera_.translation = glm::dvec3(0, 0, (properties.width * 1.5f) / properties.height);
    camera_.rotation = default_view;
}

void window::init_inputs()
{
    glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(glfw_window_, glfw_key_callback);
    glfwSetMouseButtonCallback(glfw_window_, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(glfw_window_, glfw_cursor_pos_callback);
}

void window::draw_triad()
{
    int width;
    int height;
    glfwGetFramebufferSize(glfw_window_, &width, &height);

    glViewport(0, 0, width / 5, height / 5);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, static_cast<double>(width) / height, 0.1, 30); // Todo: magic variables
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0, 0, -2); // Place triad between clipping planes.
    glRotated(camera_.rotation.x, 1, 0, 0);
    //glRotated(camera_.rotation.y, 0, 1, 0);
    glRotated(camera_.rotation.z, 0, 0, 1);

    auto const draw_axis = []() {
        GLUquadric * cylinder = gluNewQuadric();
        gluCylinder(cylinder, 0.02, 0.02, 0.8, 16, 1); // Body of axis.
        glColor3d(1,1,5); // Make arrow head white.
        glPushMatrix();
        glTranslated(0.0, 0.0, 0.8);
        gluCylinder(cylinder, 0.06, 0.001, 0.1, 12, 1); // Arrow head cone at end of axis.
        glPopMatrix();
    };

    // Z axis in blue
    glColor3d(0.3, 0.3, 1.0);
    draw_axis();

    // Y axis in green
    glColor3d(0.3, 1.0, 0.3);
    glPushMatrix();
    glRotated(-90, 1, 0, 0);
    draw_axis();
    glPopMatrix();

    // X axis in red
    glColor3d(1.0, 0.3, 0.3);
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    draw_axis();
    glPopMatrix();
}

void window::compute_projection()
{
    int width;
    int height;
    glfwGetFramebufferSize(glfw_window_, &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, static_cast<double>(width) / height, 0.1, 30); // Todo: magic variables

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(-camera_.translation.x, -camera_.translation.y, -camera_.translation.z);
    glRotated(camera_.rotation.x, 1, 0, 0);
    //glRotated(camera_.rotation.y, 0, 1, 0);
    glRotated(camera_.rotation.z, 0, 0, 1);
}

void window::glfw_key_callback(GLFWwindow * const glfw_window, int key, int scancode, int action, int modifiers)
{
    window & self = *reinterpret_cast<window *>(glfwGetWindowUserPointer(glfw_window));
    switch (action) {
        case GLFW_PRESS:    self.process_key_press(key, scancode, modifiers);   break;
        default:;
    }
}

void window::glfw_mouse_button_callback(GLFWwindow * const glfw_window, int button, int action, int modifiers)
{
    window & self = *reinterpret_cast<window *>(glfwGetWindowUserPointer(glfw_window));
    switch (action) {
        case GLFW_PRESS:    self.process_mouse_button_press(button, modifiers);     break;
        case GLFW_RELEASE:  self.process_mouse_button_release(button, modifiers);   break;
        default:;
    }
}

void window::glfw_cursor_pos_callback(GLFWwindow * const glfw_window, double xpos, double ypos)
{
    window & self = *reinterpret_cast<window *>(glfwGetWindowUserPointer(glfw_window));
    self.process_cursor_pos_change(xpos, ypos);
}

void window::process_key_press(int key, int /* scancode */, int /* modifiers */)
{
    switch(key) {
        case GLFW_KEY_ESCAPE:   glfwSetWindowShouldClose(glfw_window_, 1);  break;
        default:;
    }
}

void window::process_mouse_button_press(int button, int /* modifiers */)
{
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:    mouse_.dragging = true;     break;
        default:;
    }
}

void window::process_mouse_button_release(int button, int /* modifiers */)
{
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:    mouse_.dragging = false;    break;
        default:;
    }
}

void window::process_cursor_pos_change(double xpos, double ypos)
{
    if (mouse_.dragging) {
        double rdz = (xpos - mouse_.previous_xpos) * mouse_drag_sensitivity;
        double rdx = (ypos - mouse_.previous_ypos) * mouse_drag_sensitivity;

        camera_.rotation.z = std::clamp(camera_.rotation.z + rdz, z_axis_min, z_axis_max);
        camera_.rotation.x = std::clamp(camera_.rotation.x + rdx, x_axis_min, x_axis_max);

        //std::cout << "x: " << camera_.rotation.x << " y: " << camera_.rotation.y << " z: " << camera_.rotation.z << "\n";
    }

    mouse_.previous_xpos = xpos;
    mouse_.previous_ypos = ypos;
}

void window::process_inputs()
{
    auto update_camera = [this](int key, auto handler) {
        if (glfwGetKey(glfw_window_, key) == GLFW_PRESS)
            handler();
    };

    update_camera(GLFW_KEY_UP,
        [this](){ camera_.translation.y -= translation_step; });
    update_camera(GLFW_KEY_DOWN,
        [this](){ camera_.translation.y += translation_step; });
    update_camera(GLFW_KEY_RIGHT,
        [this](){ camera_.translation.x -= translation_step; });
    update_camera(GLFW_KEY_LEFT,
        [this](){ camera_.translation.x += translation_step; });
    update_camera(GLFW_KEY_PAGE_UP,
        [this](){ camera_.translation.z -= translation_step; });
    update_camera(GLFW_KEY_PAGE_DOWN,
        [this](){ camera_.translation.z += translation_step; });
}

} // End of namespace
