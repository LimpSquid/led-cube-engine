#include <cube/hal/mock/display.hpp>
#include <cube/hal/mock/window.hpp>
#include <cube/core/color.hpp>
#include <GL/glew.h>
#include <cmath>

using namespace cube::core;
using namespace cube::hal::mock;

#define SIZE 16

namespace
{

color c;

int map[SIZE][SIZE][SIZE];

void cls(int col)
{
    int x,y,z;
    for (x=0;x<SIZE;x++)
        for (y=0;y<SIZE;y++)
            for (z=0;z<SIZE;z++)
                map[x][y][z]=col;
}

void render_double_sinewave()
{
    static int count = 0;
    static unsigned int _timeCount1 = 0;
    static unsigned int _timeCount2 = 0;
    static double _period1 = SIZE * 1.25;
    static double _period2 = SIZE * 1.5;
    static double _sineOffset = (SIZE - 1) / 2.0;
    static double _omega1 = (2.0 * 3.14) / (double) _period1;
    static double  _omega2 = (2.0 * 3.14) / (double) _period2;

    if(count++ == 2) {
        cls(0);
        count = 0;
        // First sinewave
        for(unsigned int zAxis = _timeCount1; zAxis < (_timeCount1 + SIZE); zAxis++)
        {
            unsigned char z = round(_sineOffset * sin(zAxis * _omega1) + _sineOffset);
            unsigned char x = zAxis - _timeCount1;
            for(unsigned int yAxis = 0; yAxis < SIZE; yAxis++)
                map[x][z][yAxis] = c.r | c.g << 8;
        }

        _timeCount1++;
        if(_timeCount1 >= _period1)
            _timeCount1 = 0;

        // Second sinewave
        for(unsigned int zAxis = _timeCount2; zAxis < (_timeCount2 + SIZE); zAxis++)
        {
            unsigned char z = round(_sineOffset * sin(zAxis * _omega2) + _sineOffset);
            unsigned char x = zAxis - _timeCount2;
            for(unsigned int yAxis = 0; yAxis < SIZE; yAxis++)
                map[x][z][yAxis] = c.g << 8 | c.b << 16;
        }
        _timeCount2++;
        if(_timeCount2 >= _period2)
            _timeCount2 = 0;
    }
}

} // end of namespace

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
    // WARNING WARNING, TRASH BELOW!!!
    if (window_.close())
        exit(0);

    window_.clear();

    int x, y, z;
    float p[3],dp=1.0/float(SIZE-1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    //glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);

    glPointSize(4.0);
    glBegin(GL_POINTS);

    render_double_sinewave();

    for (p[0]=-0.5,x=0;x<SIZE;x++,p[0]+=dp)
     for (p[1]=-0.5,y=0;y<SIZE;y++,p[1]+=dp)
      for (p[2]=-0.5,z=0;z<SIZE;z++,p[2]+=dp)
        {
            // Draw "off" cube
            glColor3f(0.05, 0.05, 0.05);
            glVertex3fv(p);

            // Blend in colors
            glColor4ubv((GLubyte*)(&map[x][y][z]));
            glVertex3fv(p);
        }

    glEnd();
    glDisable(GL_BLEND);
    glPointSize(1.0);

    window_.update();
}
