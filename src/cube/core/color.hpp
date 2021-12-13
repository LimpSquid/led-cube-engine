#pragma once

#include <type_traits>
#include <limits>
#include <ostream>

namespace cube::core
{

using argb_t = uint32_t;
using color_t = unsigned char;
constexpr color_t color_min_value = std::numeric_limits<color_t>::min();
constexpr color_t color_max_value = std::numeric_limits<color_t>::max();

struct color
{
    constexpr color() :
        color(0, 0, 0, 0)
    { }

    constexpr color(argb_t argb) :
        color(argb, argb >> 8, argb >> 16, argb >> 24)
    { }

    constexpr color(
        color_t red,
        color_t green,
        color_t blue) :
        color(red, green, blue, color_max_value)
    { }

    constexpr color(
        color_t red,
        color_t green,
        color_t blue,
        color_t alpha) :
        r(red), g(green), b(blue), a(alpha)
    { }

    operator argb_t() const { return argb_t((a << 24) | (b << 16) | (g << 8) | r); }
    bool transparent() const { return a == color_min_value; }
    bool opaque() const { return a == color_max_value; }

    color_t r;
    color_t g;
    color_t b;
    color_t a;
};

inline std::ostream & operator<<(std::ostream & o, color const & c)
{
    return o
        << "r: " << int(c.r) << ", "
        << "g: " << int(c.g) << ", "
        << "b: " << int(c.b) << ", "
        << "a: " << int(c.a);
}

constexpr color color_clear {000, 000, 000, 000};
constexpr color color_white {255, 255, 255};
constexpr color color_red   {255, 000, 000};
constexpr color color_green {000, 255, 000};
constexpr color color_blue  {000, 000, 255};

}
