#pragma once

#include <cube/core/color.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <optional>

namespace cube::core
{

template<typename T>
struct json_field_converter
{
    T operator()(nlohmann::json const & json) { return T(json); }
};

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key, T def)
{
    auto const i = json.find(key);
    if (i == json.end())
        return def;
    return json_field_converter<T>{}(*i);
}

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key)
{
    using std::operator""s;

    auto const i = json.find(key);
    if (i == json.end())
        throw std::invalid_argument("Field: '"s + key + "' not present in JSON: " + json.dump());
    return json_field_converter<T>{}(*i);
}

template<typename T, typename Key>
T parse_field(nlohmann::json const & json, Key const & key, T def)
{
    return parse_field<T>(json, to_string(key), std::move(def));
}

template<typename T, typename Key>
T parse_field(nlohmann::json const & json, Key const & key)
{
    return parse_field<T>(json, to_string(key));
}

inline nlohmann::json to_json(color const & c)
{
    return {
        {"red", c.r},
        {"green", c.g},
        {"blue", c.b},
        {"alpha", c.a},
    };
}

inline color from_json(nlohmann::json const & json)
{
    auto const name = parse_field(json, "name", std::string{});
    if (name.length())
        return from_string(name);

    auto const red = parse_field<color_t>(json, "red");
    auto const green = parse_field<color_t>(json, "green");
    auto const blue = parse_field<color_t>(json, "blue");
    auto const alpha = parse_field<color_t>(json, "alpha", color_max_value);

    return {red, green, blue, alpha};
}

template<>
struct json_field_converter<color>
{
    color operator()(nlohmann::json const & json) { return from_json(json); }
};

} // End of namespace
