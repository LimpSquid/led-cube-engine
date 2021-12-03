#include <cube/hal/opengl/mock_display.h>
#include <cube/hal/opengl/window.h>
#include <cube/core/color.h>
#include <GL/glew.h>

using namespace cube::core;
using namespace cube::hal::opengl;

namespace
{
    color c;
}

mock_display::mock_display() :
    window_(window::instance())
{

}

mock_display::~mock_display()
{

}

void mock_display::show(graphics_buffer & buffer)
{
    // Todo: temporary show color
    c = buffer.test_color;
}

void mock_display::poll()
{
    if (window_.close())
        exit(0);

    window_.clear();

    glBegin(GL_QUADS);
    glColor3f(c.r / 255.0, c.g / 255.0, c.b / 255.0);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(-0.5f,  0.5f);
    glVertex2f( 0.5f,  0.5f);
    glVertex2f( 0.5f, -0.5f);
    glEnd();

    window_.update();
}
