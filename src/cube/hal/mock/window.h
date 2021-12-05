#pragma once

#include <glm/vec3.hpp>

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
    window(window_properties properties);

    void init_window(window_properties const & properties);
    void init_inputs();

    void process_inputs();

    GLFWwindow * glfw_window_;
    glm::vec3 camera_;
};

}
