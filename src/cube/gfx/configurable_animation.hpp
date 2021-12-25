#pragma once

#include <cube/gfx/property_value.hpp>
#include <cube/core/animation.hpp>
#include <boost/preprocessor.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <unordered_map>

// Expands to `"enum_element"` for index 0, all other indices this expands to `, "enum_element"`
#define PROPERTY_ENUM_PROCESS_ONE(r, unused, index, element) BOOST_PP_COMMA_IF(index) BOOST_PP_STRINGIZE(element)
#define PROPERTY_ENUM(...) \
    enum property : configurable_animation::property_label_type { __VA_ARGS__ }; \
    friend char const * const to_string(property value) \
    { \
        static const char * const strings[] = \
        { \
            BOOST_PP_SEQ_FOR_EACH_I(PROPERTY_ENUM_PROCESS_ONE, %%, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
        }; \
        return strings[value]; \
    }

namespace cube::gfx
{

class configurable_animation :
    public core::animation
{
public:
    using property_label_type = int;
    using property_pair = std::pair<property_label_type, property_value>;

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
    void write_properties(nlohmann::json const & properties);

protected:
    configurable_animation(core::engine_context & context, char const * const name);

private:
    virtual std::vector<property_pair> parse(nlohmann::json const & json) const;

    std::unordered_map<property_label_type, std::string> properties_;
};

} // end of namespace
