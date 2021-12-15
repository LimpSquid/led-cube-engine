#pragma once

#include <limits>
#include <cmath>
#include <ostream>

namespace cube::core
{

using rgba_t = uint32_t;
using color_t = unsigned char;
constexpr color_t color_min_value = std::numeric_limits<color_t>::min();
constexpr color_t color_max_value = std::numeric_limits<color_t>::max();

struct color
{
    constexpr color() :
        color(0, 0, 0, 0)
    { }

    explicit constexpr color(rgba_t rgba) :
        color(rgba, rgba >> 8, rgba >> 16, rgba >> 24)
    { }

    constexpr color(color_t red, color_t green, color_t blue) :
        color(red, green, blue, color_max_value)
    { }

    constexpr color(color_t red, color_t green, color_t blue, color_t alpha) :
        r(red), g(green), b(blue), a(alpha)
    { }

    rgba_t rgba() const { return rgba_t(r | (g << 8) | (b << 16) | (a << 24)); }
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
        << "r: " << int(c.r) << ", " << "g: " << int(c.g) << ", "
        << "b: " << int(c.b) << ", " << "a: " << int(c.a);
}

inline constexpr color operator!(color const & c)
{
    color_t r = color_max_value - c.r;
    color_t g = color_max_value - c.g;
    color_t b = color_max_value - c.b;

    return { r, g, b, c.a};
}

inline bool operator==(color const & lhs, color const & rhs) { return lhs.rgba() == rhs.rgba(); }
inline bool operator!=(color const & lhs, color const & rhs) { return lhs.rgba() != rhs.rgba(); }

constexpr color color_clear     {000, 000, 000, 000};
constexpr color color_black     {000, 000, 000};
constexpr color color_white     {255, 255, 255};
constexpr color color_red       {255, 000, 000};
constexpr color color_green     {000, 255, 000};
constexpr color color_blue      {000, 000, 255};
constexpr color color_cyan      {000, 255, 255};
constexpr color color_magenta   {255, 000, 255};
constexpr color color_yellow    {255, 255, 000};
constexpr color color_orange    {255, 128, 000};

struct color_scalar
{
    color_scalar(double red, double green, double blue, double alpha) :
        r(red), g(green), b(blue), a(alpha)
    { }

    color_scalar(color const & c) :
        color_scalar(c.r, c.g, c.b, c.a)
    { }

    operator color() const
    {
        auto clamp = [](double scalar) {
            int x = round(scalar);
            if (x < color_min_value) return color_min_value;
            if (x > color_max_value) return color_max_value;
            return color_t(x);
        };

        return {clamp(r), clamp(g), clamp(b), clamp(a)};
    }

    double r;
    double g;
    double b;
    double a;
};

color_scalar operator+(color_scalar const & lhs, color_scalar const & rhs);
color_scalar operator-(color_scalar const & lhs, color_scalar const & rhs);
color_scalar operator*(color_scalar const & lhs, double scalar);
color_scalar operator/(color_scalar const & lhs, double scalar);

void alpha_blend(color const & c, color & bucket);
void blend(color const & c, color & bucket);
void blend(color const & c, rgba_t & bucket);

}
