#pragma once

#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <chrono>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace cube::core { class engine_context; }
namespace cube::gfx
{

template<typename T>
struct property_value_converter
{
    static std::string convert(T const & value)
    {
        std::stringstream stream;
        stream << value;
        return stream.str();
    }

    static T convert(std::string const & value)
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
    static std::string convert(std::string const & value) { return value; }
};

template<class Rep, class Period>
struct property_value_converter<std::chrono::duration<Rep, Period>>
{
    static std::string convert(std::chrono::duration<Rep, Period> const & value)
    {
        return property_value_converter<Rep>::convert(value.count());
    }

    static std::chrono::duration<Rep, Period> convert(std::string const & value)
    {
        auto const count = property_value_converter<Rep>::convert(value);
        return std::chrono::duration<Rep, Period>(count);
    }
};

template<>
struct property_value_converter<core::color>
{
    static std::string convert(core::color const & value)
    {
        std::stringstream stream;
        stream << value.r << value.g << value.b << value.a;
        return stream.str();
    }

    static core::color convert(std::string const & value)
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
        property(property_value_converter<T>::convert(v))
    { }

    std::string const property;
};

template<>
struct property_value_converter<property_value>
{
    static std::string convert(property_value const & value) { return value.property; }
};

class configurable_animation :
    public core::animation
{
public:
    using property_label_type = int;

    enum : property_label_type
    {
        property_custom     = 255, // First usable label for custom properties
    };

    template<typename T>
    void write_property(property_label_type label, T value)
    {
        properties_[label] = property_value_converter<T>::convert(value);
    }

    void write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties);

    template<typename T>
    T read_property(property_label_type label, T def = T()) const
    {
        auto const search = properties_.find(label);

        if (search == properties_.end() || search->second.empty())
            return def;

        // Todo: maybe do some checking if we are able to convert the string to the given type.
        return property_value_converter<T>::convert(search->second);
    }

protected:
    configurable_animation(core::engine_context & context);

    core::engine_context & context();

private:
    core::engine_context & context_;
    std::unordered_map<property_label_type, std::string> properties_;
};

} // end of namespace
