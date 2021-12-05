#pragma once

#include <type_traits>
#include <limits>

namespace cube::core
{

struct color
{
    using value_type = unsigned char;
    using min_value = std::integral_constant<value_type, std::numeric_limits<value_type>::min()>;
    using max_value = std::integral_constant<value_type, std::numeric_limits<value_type>::max()>;

    constexpr color() :
        color(0, 0, 0, 0)
    { }

    constexpr color(
        value_type red,
        value_type green,
        value_type blue) :
        color(red, green, blue, max_value::value)
    { }

    constexpr color(
        value_type red,
        value_type green,
        value_type blue,
        value_type alpha) :
        r(red), g(green), b(blue), a(alpha)
    { }

    value_type r;
    value_type g;
    value_type b;
    value_type a;
};

using color_t = color::value_type;
constexpr color_t color_min_value = color::min_value::value;
constexpr color_t color_max_value = color::max_value::value;

}
