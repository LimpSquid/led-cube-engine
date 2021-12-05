#include <cube/hal/mock/window.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace cube::hal::mock;

namespace
{
    void glfw_error_callback(int, const char * const desc)
    {
        std::cerr << "GLFW error: " << desc << "\n";
    }

    struct V { GLdouble x, y, z; };

    double norm(V v){ return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

    V operator-(V a, V b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
    V operator+(V a, V b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
    V operator*(V v, double s) { return {v.x * s, v.y * s, v.z * s}; }
    V & operator+=(V & a, V b) { return a = a + b; }
    V & operator-=(V & a, V b) { return a = a - b; }

    V operator/(V v, double s) { return {v.x / s, v.y / s, v.z / s}; }
    V normalize(V v){ return v / norm(v); }

    V camera{0, 0, 2};
    double rotation = 0;
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
    int width, height;
    glfwGetFramebufferSize(glfw_window_, &width, &height);
    float ratio = width / (float) height;
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ratio, 0.1, 30);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glTranslatef(-camera.x, -camera.y, -camera.z);

    if (glfwGetKey(glfw_window_, GLFW_KEY_UP) == GLFW_PRESS)
        camera.y += 0.02;
    if (glfwGetKey(glfw_window_, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.y -= 0.02;
    if (glfwGetKey(glfw_window_, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.x -= 0.02;
    if (glfwGetKey(glfw_window_, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.x += 0.02;
    if (glfwGetKey(glfw_window_, GLFW_KEY_HOME) == GLFW_PRESS)
        camera.z -= 0.02;
    if (glfwGetKey(glfw_window_, GLFW_KEY_END) == GLFW_PRESS)
        camera.z += 0.02;

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
