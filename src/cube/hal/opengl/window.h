#pragma once

struct GLFWwindow;

namespace cube::hal::opengl
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
    window(window_properties properties);

    void window_init(window_properties const & properties);
    void input_init();

    GLFWwindow * glfw_window_;
};

}
