#pragma once

#include <cube/core/color.hpp>
#include <string>
#include <chrono>
#include <sstream>

namespace cube::gfx
{

template<typename T>
struct property_value_converter
{
    std::string operator()(T const & value)
    {
        std::stringstream stream;
        stream << value;
        return stream.str();
    }

    T operator()(std::string const & value)
    {
        T result;
        std::stringstream stream(value);
        stream >> result;
        return result;
    }
};

template<>
struct property_value_converter<std::string>
{
    std::string operator()(std::string const & value) { return value; }
};

template<class Rep, class Period>
struct property_value_converter<std::chrono::duration<Rep, Period>>
{
    std::string operator()(std::chrono::duration<Rep, Period> const & value)
    {
        return property_value_converter<Rep>{}(value.count());
    }

    std::chrono::duration<Rep, Period> operator()(std::string const & value)
    {
        auto const count = property_value_converter<Rep>{}(value);
        return std::chrono::duration<Rep, Period>(count);
    }
};

template<>
struct property_value_converter<core::color>
{
    std::string operator()(core::color const & value)
    {
        std::stringstream stream;
        stream << value.r << value.g << value.b << value.a;
        return stream.str();
    }

    core::color operator()(std::string const & value)
    {
        core::color_t r, g, b, a;
        std::stringstream stream(value);
        stream >> r >> g >> b >> a;
        return {r, g, b, a};
    }
};

struct property_value
{
    template<typename T>
    property_value(T const & v) :
        property(property_value_converter<T>{}(v))
    { }

    std::string const property;
};

template<>
struct property_value_converter<property_value>
{
    std::string operator()(property_value const & value) { return value.property; }
};

} // End of namespace
