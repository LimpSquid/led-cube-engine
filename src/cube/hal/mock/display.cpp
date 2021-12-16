#include <cube/hal/mock/display.hpp>
#include <cube/hal/mock/window.hpp>
#include <cube/core/color.hpp>
#include <GL/glew.h>
#include <cmath>
#include <cstring>

using namespace cube::core;

namespace cube::hal::mock
{

display::display() :
    window_(window::instance())
{
}

display::~display()
{

}

void display::show(graphics_buffer const & buffer)
{
    buffer_ = buffer;
}

void display::poll()
{
    if (window_.close())
        exit(0);
    window_.clear();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glPointSize(4);
    glBegin(GL_POINTS);

    float pos[3];
    float pos_step = 1.0 / (cube_size_1d - 1);
    int x, y, z;
    for (x = 0, pos[0] = -0.5; x < cube_size_1d; x++, pos[0] += pos_step) {
        for (y = 0, pos[1] = -0.5; y < cube_size_1d; y++, pos[1] += pos_step) {
            for (z = 0, pos[2] = -0.5; z < cube_size_1d; z++, pos[2] += pos_step) {
                // Draw "off" cube
                glColor3f(0.08, 0.08, 0.08);
                glVertex3fv(pos);

                // Blend in colors
                int offset = map_to_offset(x, y, z);
                glColor3ubv(reinterpret_cast<GLubyte const *>(&buffer_.data[offset]));
                glVertex3fv(pos);
            }
        }
    }

    glEnd();
    glDisable(GL_BLEND);

    window_.update();
}

} // End of namespace
