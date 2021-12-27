#pragma once

#include <cube/gfx/property_value.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/enum.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <unordered_map>

#define PROPERTY_ENUM(...) ENUM(property, property_label_type, 255, __VA_ARGS__)
#define PROPERTY_ENUM_SHARED(...) \
    static_assert(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__) < 255); \
    ENUM(property, property_label_type, 0, __VA_ARGS__)

namespace cube::gfx
{

class configurable_animation :
    public core::animation
{
public:
    using property_label_type = int;
    using property_pair = std::pair<property_label_type, property_value>;

    PROPERTY_ENUM_SHARED
    (
        animation_label,
    )

    template<typename T>
    void write_property(property_label_type label, T value)
    {
        properties_[label] = property_value_converter<T>{}(value);
    }

    template<typename T>
    T read_property(property_label_type label, T def = T()) const
    {
        auto const search = properties_.find(label);

        if (search == properties_.end() || search->second.empty())
            return def;

        // Todo: maybe do some checking if we are able to convert the string to the given type.
        return property_value_converter<T>{}(search->second);
    }

    void write_properties(std::vector<property_pair> const & properties);
    void load_properties(nlohmann::json const & json);

protected:
    configurable_animation(core::engine_context & context);

private:
    virtual std::vector<property_pair> parse(nlohmann::json const & json) const = 0;

    std::unordered_map<property_label_type, std::string> properties_;
};

} // end of namespace
