#pragma once

#include <type_traits>

namespace cube::util
{

template<typename ValueType>
struct basic_color
{
    using value_type = ValueType;

    basic_color() : basic_color(0, 0, 0) { }
    basic_color(value_type r, value_type g, value_type b) :
        red(r), green(g), blue(b) { }
    basic_color(const basic_color &other) = default;
    ~basic_color() = default;

    basic_color &operator=(const basic_color &other) = default;

    value_type red;
    value_type green;
    value_type blue;

private:
    static_assert(std::is_integral<value_type>::value, "Value type is not integral");
};

}