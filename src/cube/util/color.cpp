#include <util/color.h>

using namespace cube::util;

color::color() :
    color(0, 0, 0)
{

}

color::color(unsigned char red, unsigned char green, unsigned char blue) :
    red_(red),
    green_(green),
    blue_(blue)
{

}