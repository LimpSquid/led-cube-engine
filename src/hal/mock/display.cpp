#include <hal/mock/display.hpp>
#include <hal/mock/window.hpp>
#include <GL/glew.h>
#include <boost/asio/post.hpp>
#include <cstring>

using namespace cube;
using namespace cube::core;
using namespace boost;

namespace
{

constexpr double cube_off_brightness = 20.5 / (cube_size_1d * cube_size_1d);
constexpr hal::mock::window_properties window_resolution = {960, 720};

} // End of namespace

namespace hal::mock
{

display::display(engine_context & context) :
    graphics_device(context),
    window_(window::instance(window_resolution)),
    tracker_(make_parent_tracker())
{
    schedule_update();
}

display::~display()
{ }

void display::show(graphics_buffer const & buffer)
{
    buffer_ = buffer;
}

void display::schedule_update()
{
    asio::post(io_context().get_executor(), [this, t = weak(tracker_)]() {
        if (parent_in_scope(t)) // In case display gets destroyed and the handler has yet to be executed
            update();
    });
}

void display::update()
{
    if (window_.close())
        exit(0);
    window_.clear();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glPointSize(4);
    glBegin(GL_POINTS);

    double pos[3];
    double pos_step = 1.0 / (cube_size_1d - 1);
    int x, y, z;
    for (x = 0, pos[0] = -0.5; x < cube_size_1d; x++, pos[0] += pos_step) {
        for (y = 0, pos[1] = -0.5; y < cube_size_1d; y++, pos[1] += pos_step) {
            for (z = 0, pos[2] = -0.5; z < cube_size_1d; z++, pos[2] += pos_step) {
                // Draw "off" cube
                glColor3d(cube_off_brightness, cube_off_brightness, cube_off_brightness);
                glVertex3dv(pos);

                // Blend in colors
                int offset = map_to_offset(x, y, z);
                glColor3ubv(reinterpret_cast<GLubyte const *>(&buffer_.data[offset]));
                glVertex3dv(pos);
            }
        }
    }

    glEnd();
    glDisable(GL_BLEND);

    window_.update();

    // Schedule next update
    schedule_update();
}

} // End of namespace
