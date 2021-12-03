#include <cube/hal/mock/display.h>
#include <cube/hal/mock/window.h>
#include <cube/core/color.h>
#include <GL/glew.h>

using namespace cube::core;
using namespace cube::hal::mock;

namespace
{
    color c;
}

display::display() :
    window_(window::instance())
{

}

display::~display()
{

}

void display::show(graphics_buffer & buffer)
{
    // Todo: temporary show color
    c = buffer.test_color;
}

void display::poll()
{
    if (window_.close())
        exit(0);

    window_.clear();

    glBegin(GL_QUADS);
    for (double i = 255.0; i < 510.0; i += 1.0) {
        glColor3f(c.r / i, c.g / i, c.b / i);
        glVertex2f(-c.r / i, -c.r / i);
        glVertex2f(-c.g / i,  c.g / i);
        glVertex2f( c.b / i,  c.b / i);
        glVertex2f( c.b / i, -c.b / i);
    }
    glEnd();

    window_.update();
}
