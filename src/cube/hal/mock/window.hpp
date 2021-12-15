#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp> // @Todo: not sure if we keep this here

struct GLFWwindow;

namespace cube::hal::mock
{

struct window_properties
{
    char const * const title{"mock"};
    int width{1024};
    int height{768};
};

class window
{

public:
    static window & instance(window_properties creation_properties = {});
    ~window();

    void clear();
    void update();
    bool close();

private:
    struct camera
    {
        glm::vec3 translation;
        glm::vec3 rotation;
    };

    window(window_properties properties);

    void init_window(window_properties const & properties);
    void init_inputs();

    static void glfw_key_callback(GLFWwindow * const glfw_window, int key, int scancode, int action, int modifiers);
    void process_key_press(int key, int scancode, int modifiers);
    void process_inputs();

    GLFWwindow * glfw_window_;
    camera camera_;
};

} // end of namespace
