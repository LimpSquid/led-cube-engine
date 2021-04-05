#pragma once

#include <type_traits>

namespace cube::util
{

template<typename ColorType>
struct color
{
    using value_type = ColorType;

    color() : color(0, 0, 0) { }
    color(value_type r, value_type g, value_type b) :
        red(r), green(g), blue(b)
    {
        static_assert(std::is_integral<value_type>::value, "Color type is not integral");
    }
    color(const color &other) = default;
    ~color() = default;

    color &operator=(const color &other) = default;

    value_type red;
    value_type green;
    value_type blue;
};

using color_uchar = color<unsigned char>;

}