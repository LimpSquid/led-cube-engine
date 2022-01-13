#pragma once

#include <3rdparty/glm/vec4.hpp>
#include <limits>
#include <string>

namespace cube::core
{

using rgba_t = uint32_t;
using color_t = unsigned char;
using color_vec_t = glm::dvec4;
constexpr color_t color_min_value{std::numeric_limits<color_t>::min()};
constexpr color_t color_max_value{std::numeric_limits<color_t>::max()};

struct color
{
    constexpr color() :
        color(0, 0, 0, 0)
    { }

    constexpr color(rgba_t rgba) :
        color(color_t(rgba), color_t(rgba >> 8), color_t(rgba >> 16), color_t(rgba >> 24))
    { }

    constexpr color(color_t red, color_t green, color_t blue) :
        color(red, green, blue, color_max_value)
    { }

    constexpr color(color_t red, color_t green, color_t blue, color_t alpha) :
        r(red), g(green), b(blue), a(alpha)
    { }

    color(color_vec_t const & vec);

    constexpr operator color_vec_t() const { return {r, g, b, a}; }
    constexpr color_vec_t vec() const { return *this; }
    constexpr rgba_t rgba() const { return rgba_t(r | (g << 8) | (b << 16) | (a << 24)); }
    constexpr bool transparent() const { return a == color_min_value; }
    constexpr bool opaque() const { return a == color_max_value; }

    color_t r;
    color_t g;
    color_t b;
    color_t a;
};

inline constexpr color operator!(color const & c)
{
    color_t r = static_cast<color_t>(color_max_value - c.r);
    color_t g = static_cast<color_t>(color_max_value - c.g);
    color_t b = static_cast<color_t>(color_max_value - c.b);

    return {r, g, b, c.a};
}

constexpr color color_transparent   {000, 000, 000, 000};
constexpr color color_black         {000, 000, 000};
constexpr color color_white         {255, 255, 255};
constexpr color color_red           {255, 000, 000};
constexpr color color_green         {000, 255, 000};
constexpr color color_blue          {000, 000, 255};
constexpr color color_cyan          {000, 255, 255};
constexpr color color_magenta       {255, 000, 255};
constexpr color color_yellow        {255, 255, 000};
constexpr color color_orange        {255, 128, 000};
constexpr color color_pink          {255,  96, 255};

constexpr inline color_vec_t red_vec(double scalar) { return {scalar, 1.0, 1.0, 1.0}; }
constexpr inline color_vec_t green_vec(double scalar) { return {1.0, scalar, 1.0, 1.0}; }
constexpr inline color_vec_t blue_vec(double scalar) { return {1.0, 1.0, scalar, 1.0}; }
constexpr inline color_vec_t alpha_vec(double scalar) { return {1.0, 1.0, 1.0, scalar}; }
constexpr inline color_vec_t rgb_vec(double scalar) { return {scalar, scalar, scalar, 1.0}; }
constexpr inline color_vec_t rgba_vec(double scalar) { return {scalar, scalar, scalar, scalar}; }

inline bool operator==(color const & lhs, color const & rhs) { return lhs.rgba() == rhs.rgba(); }
inline bool operator!=(color const & lhs, color const & rhs) { return lhs.rgba() != rhs.rgba(); }

inline void scale(rgba_t & rgba, color_vec_t scalar)
{
    auto scaled = color(rgba).vec() * scalar;
    rgba = color(scaled).rgba();
}

inline void scale(rgba_t & rgba, double scalar)
{
    return scale(rgba, rgba_vec(scalar));
}

inline color random_color(bool opaque = true)
{
    auto const crand = []() { return static_cast<color_t>(std::rand() % color_max_value); };
    return {crand(), crand(), crand(), opaque ? color_max_value : crand()};
}

constexpr inline bool opaque(rgba_t const & rgba)
{
    return color(rgba).opaque();
}

color from_string(std::string c);
void alpha_blend(color const & c, color & bucket);
void alpha_blend(rgba_t const & c, rgba_t & bucket);
void blend(color const & c, color & bucket);
void blend(rgba_t const & c, rgba_t & bucket);
void blend(color const & c, rgba_t & bucket);

} // End of namespace
