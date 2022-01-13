#pragma once

#include <cube/core/color.hpp>
#include <cube/core/expression.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/type_index.hpp>

namespace cube::core
{

template<typename T>
struct json_value_converter
{
    T operator()(nlohmann::json const & json)
    {
        using std::operator""s;

#ifdef EVAL_EXPRESSION
        if (json.is_string()) {
            if constexpr (std::is_integral_v<T>) {
                double const value = std::round(evald(json));
                if (value > static_cast<double>(std::numeric_limits<T>::max()) ||
                    value < static_cast<double>(std::numeric_limits<T>::min()))
                    throw std::overflow_error("Expression eval result overflowed for integral type: "s +
                        boost::typeindex::type_id<T>().pretty_name());
                return static_cast<T>(value);
            } else if constexpr (std::is_floating_point_v<T>)
                return eval<T>(json);
        }
#endif
        return T(json);
    }

    nlohmann::json operator()(T const & value) { return value; }
};

template<>
struct json_value_converter<nlohmann::json>
{
    nlohmann::json operator()(nlohmann::json const & json) { return json; }
};

template<class Rep, class Period>
struct json_value_converter<std::chrono::duration<Rep, Period>>
{
    std::chrono::duration<Rep, Period> operator()(nlohmann::json const & json)
    {
        auto const count = json_value_converter<Rep>{}(json);
        return std::chrono::duration<Rep, Period>(count);
    }

    nlohmann::json operator()(std::chrono::duration<Rep, Period> const & value)
    {
        return json_value_converter<Rep>{}(value.count());
    }
};

template<typename T> nlohmann::json to_json(T const &);
template<typename T> T from_json(nlohmann::json const &);
template<typename T>
struct json_value_converter<std::vector<T>>
{
    std::vector<T> operator()(nlohmann::json const & json)
    {
        using std::operator""s;

        if (!json.is_array())
            throw std::invalid_argument("Expected JSON array got: "s + json.type_name());

        std::vector<T> result;
        for (auto const & element : json)
            result.push_back(from_json<T>(element));
        return result;
    }

    nlohmann::json operator()(std::vector<T> const & value)
    {
        auto array = nlohmann::json::array();
        for (auto const & element : value)
            array.insert(array.end(), to_json(element));
        return array;
    }
};

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key, T def)
{
    using std::operator""s;

    auto const i = json.find(key);
    if (i == json.end())
        return def;

    try {
        return json_value_converter<T>{}(*i);
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to convert field: '"s + key
            + "' in JSON: " + json.dump() + ", error: " + ex.what());
    }
}

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key)
{
    using std::operator""s;

    auto const i = json.find(key);
    if (i == json.end())
        throw std::invalid_argument("Field: '"s + key + "' not present in JSON: " + json.dump());

    try {
        return json_value_converter<T>{}(*i);
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to convert field: '"s + key
            + "' in JSON: " + json.dump() + ", error: " + ex.what());
    }
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

template<typename T>
nlohmann::json make_field(char const * const key, T const & value)
{
    using std::operator""s;

    try {
        return {key, json_value_converter<T>{}(value)};
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to dump field: '"s + key
            + "', error: " + ex.what());
    }
}

template<typename T, typename Key>
nlohmann::json make_field(Key const & key, T const & value)
{
    return make_field<T>(to_string(key), value);
}

template<>
inline nlohmann::json to_json(color const & c)
{
    return {
        {"red", c.r},
        {"green", c.g},
        {"blue", c.b},
        {"alpha", c.a},
    };
}

template<>
inline color from_json(nlohmann::json const & json)
{
    auto const name = parse_field(json, "name", std::string{});
    if (name.length()) {
        return name == "random_color"
            ? random_color()
            : from_string(name);
    }

    auto const red = parse_field<color_t>(json, "red", color_min_value);
    auto const green = parse_field<color_t>(json, "green", color_min_value);
    auto const blue = parse_field<color_t>(json, "blue", color_min_value);
    auto const alpha = parse_field<color_t>(json, "alpha", color_max_value);

    return {red, green, blue, alpha};
}

template<>
struct json_value_converter<color>
{
    color operator()(nlohmann::json const & json) { return from_json<color>(json); }
    nlohmann::json operator()(color const & value) { return to_json<color>(value); }
};

} // End of namespace
