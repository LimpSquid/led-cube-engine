#pragma once

#include <type_traits>
#include <numeric>

namespace cube::util
{

template<typename ValueType,
    ValueType DefaultRed = std::numeric_limits<ValueType>::min(),
    ValueType DefaultGreen = std::numeric_limits<ValueType>::min(),
    ValueType DefaultBlue = std::numeric_limits<ValueType>::min(),
    ValueType DefaultAlpha = std::numeric_limits<ValueType>::max()>
struct basic_color
{
    using value_type = ValueType;

    basic_color() :
        basic_color(DefaultRed, DefaultGreen, DefaultBlue, DefaultAlpha) { }
    basic_color(value_type r, value_type g, value_type b) :
        basic_color(r, g, b, DefaultAlpha) { }
    basic_color(value_type r, value_type g, value_type b, value_type a) :
        red(r), green(g), blue(b), alpha(a) { }
    basic_color(const basic_color &other) = default;
    ~basic_color() = default;

    basic_color &operator=(const basic_color &other) = default;

    value_type red;
    value_type green;
    value_type blue;
    value_type alpha;

private:
    static_assert(std::is_integral<value_type>::value, "Value type is not integral");
};

}