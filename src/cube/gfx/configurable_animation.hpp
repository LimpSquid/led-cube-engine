#pragma once

#include <cube/gfx/property_value.hpp>
#include <cube/core/animation.hpp>
#include <vector>
#include <unordered_map>

namespace cube::gfx
{

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
        properties_[label] = property_value_converter<T>{}(value);
    }

    void write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties);

    template<typename T>
    T read_property(property_label_type label, T def = T()) const
    {
        auto const search = properties_.find(label);

        if (search == properties_.end() || search->second.empty())
            return def;

        // Todo: maybe do some checking if we are able to convert the string to the given type.
        return property_value_converter<T>{}(search->second);
    }

protected:
    configurable_animation(core::engine_context & context);

private:
    std::unordered_map<property_label_type, std::string> properties_;
};

} // end of namespace
