#include <cube/hal/mock/display.h>
#include <cube/hal/mock/window.h>
#include <cube/core/color.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

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

    int color = (int)c.b << 16 | (int)c.g << 8 | (int)c.r;
    cls(color);
}

void display::poll()
{
    if (window_.close())
        exit(0);

    window_.clear();

    int x, y, z;
    float p[3],dp=1.0/float(SIZE-1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);

    glPointSize(3.0);
    glBegin(GL_POINTS);

    for (p[0]=-0.5,x=0;x<SIZE;x++,p[0]+=dp)
     for (p[1]=-0.5,y=0;y<SIZE;y++,p[1]+=dp)
      for (p[2]=-0.5,z=0;z<SIZE;z++,p[2]+=dp)
        {
            glColor4ubv((GLubyte*)(&map[x][y][z]));
            glVertex3fv(p);
        }

    glEnd();
    glDisable(GL_BLEND);
    glPointSize(1.0);

    window_.update();
}
