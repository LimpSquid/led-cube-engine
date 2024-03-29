#include <driver/mock/display.hpp>
#include <driver/mock/window.hpp>
#include <GL/glew.h>
#include <boost/asio/post.hpp>
#include <cstring>

using namespace cube;
using namespace cube::core;
using namespace boost;

namespace
{

constexpr double point_size{std::max(1.75, 80.0 / cube_size_1d)};
constexpr double cube_off_brightness{0.75 / cube_size_1d};
constexpr driver::mock::window_properties window_resolution{960, 720};

} // End of namespace

namespace driver::mock
{

display::display(engine_context & context) :
    graphics_device(context),
    window_(window::instance(window_resolution)),
    invoker_(context.event_poller, std::bind(&display::update, this))
{
    invoker_.schedule();
}

void display::show(graphics_buffer const & buffer)
{
    buffer_ = buffer;
}

void display::update()
{
    window_.clear();

    if (window_.close()) {
        raise(SIGINT);
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glPointSize(point_size);
    glBegin(GL_POINTS);

    double pos[3];
    double pos_step = 1.0 / (cube_size_1d - 1);
    int x, y, z;
    rgba_t rgba;
    for (x = 0, pos[0] = -0.5; x < cube_size_1d; x++, pos[0] += pos_step) {
        for (y = 0, pos[1] = -0.5; y < cube_size_1d; y++, pos[1] += pos_step) {
            for (z = 0, pos[2] = -0.5; z < cube_size_1d; z++, pos[2] += pos_step) {
                // Draw "off" cube
                glColor3d(cube_off_brightness, cube_off_brightness, cube_off_brightness);
                glVertex3dv(pos);

                // Blend in colors
                rgba = buffer_.data[map_to_offset(x, y, z)];
                glColor3ub(
                    static_cast<GLubyte>(rgba >> 24),
                    static_cast<GLubyte>(rgba >> 16),
                    static_cast<GLubyte>(rgba >> 8)); // Fixme: not as performant as it could be, but its endianness safe
                glVertex3dv(pos);
            }
        }
    }

    glEnd();
    glDisable(GL_BLEND);

    window_.update();
    invoker_.schedule(); // Safe as window_.update() is blocking (bound to display's refresh rate)
}

} // End of namespace
