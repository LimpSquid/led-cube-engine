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

    struct mouse
    {
        double previous_xpos{0.0};
        double previous_ypos{0.0};
        bool dragging{false};
    };

    window(window_properties properties);

    void init_window(window_properties const & properties);
    void init_inputs();

    void draw_triad();
    void compute_projection();

    static void glfw_key_callback(GLFWwindow * const glfw_window, int key, int scancode, int action, int modifiers);
    static void glfw_mouse_button_callback(GLFWwindow * const glfw_window, int button, int action, int modifiers);
    static void glfw_cursor_pos_callback(GLFWwindow * const glfw_window, double xpos, double ypos);
    void process_key_press(int key, int scancode, int modifiers);
    void process_mouse_button_press(int button, int modifiers);
    void process_mouse_button_release(int button, int modifiers);
    void process_cursor_pos_change(double xpos, double ypos);
    void process_inputs();

    GLFWwindow * glfw_window_;
    camera camera_;
    mouse mouse_;
};

} // End of namespace
