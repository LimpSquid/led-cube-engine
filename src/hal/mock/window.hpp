#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct GLFWwindow;

namespace hal::mock
{

struct window_properties
{
    constexpr window_properties(int w, int h, char const * const t) :
        width(w),
        height(h),
        title(t)
    { }

    constexpr window_properties(int w, int h) :
        window_properties(w, h, "mock")
    { }

    constexpr window_properties() :
        window_properties(1024, 768, "mock")
    { }

    char const * const title;
    int width;
    int height;
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
        glm::dvec3 translation;
        glm::dvec3 rotation;
    };

    struct mouse
    {
        glm::dvec2 previous_cursor_pos{0, 0};
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
    static void glfw_scroll_callback(GLFWwindow * const glfw_window, double xoffset, double yoffset);
    void process_key_press(int key, int scancode, int modifiers);
    void process_mouse_button_press(int button, int modifiers);
    void process_mouse_button_release(int button, int modifiers);
    void process_cursor_pos_change(double xpos, double ypos);
    void process_scroll_change(double xoffset, double yoffset);
    void process_inputs();

    GLFWwindow * glfw_window_;
    camera camera_;
    mouse mouse_;
};

} // End of namespace
