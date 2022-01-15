#pragma once

#include <cube/core/color.hpp>
#include <cube/core/expression.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/type_index.hpp>

namespace cube::core
{

template<typename T, typename = void>
struct is_json_iterable : std::false_type {};
template<typename T>
struct is_json_iterable<T,
    std::void_t<
        // Iterators must be present
        decltype(std::begin(std::declval<T>())),
        decltype(std::end(std::declval<T>())),
        // The object type of the container must be convertable to/from JSON
        decltype(to_json(std::declval<typename T::value_type>(), std::declval<nlohmann::json&>())),
        decltype(from_json(std::declval<nlohmann::json>(), std::declval<typename T::value_type&>()))>
    > : std::true_type {};
template<typename T>
constexpr bool is_json_iterable_v = is_json_iterable<T>::value;

template<typename T>
void to_json(T const & value, nlohmann::json & out)
{
    if constexpr (is_json_iterable_v<T> && !std::is_same_v<T, std::string>) {
        nlohmann::json element_out;
        out = nlohmann::json::array();
        for (auto const & element : value) {
            to_json(element, element_out);
            out.insert(out.end(), std::move(element_out));
        }
    } else
        out = value;
}

template<typename T>
void from_json(nlohmann::json const & json, T & out)
{
    if constexpr (is_json_iterable_v<T> && !std::is_same_v<T, std::string>) {
        using std::operator""s;

        if (!json.is_array())
            throw std::invalid_argument("Expected JSON array got: "s + json.type_name());

        typename T::value_type element_out;
        for (auto const & element : json) {
            from_json(element, element_out);
            out.push_back(std::move(element_out));
        }
    } else
        out = T(json);
}

template<typename T>
struct json_value_converter
{
    T operator()(nlohmann::json const & json)
    {
#ifdef EVAL_EXPRESSION
        using std::operator""s;

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
        T out;
        from_json(json, out);
        return out;
    }

    nlohmann::json operator()(T const & value)
    {
        nlohmann::json out;
        to_json(value, out);
        return out;
    }
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

template<typename T>
std::optional<T> parse_optional_field(nlohmann::json const & json, char const * const key)
{
    using std::operator""s;

    auto const i = json.find(key);
    if (i == json.end())
        return {};

    try {
        return {json_value_converter<T>{}(*i)};
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to convert field: '"s + key
            + "' in JSON: " + json.dump() + ", error: " + ex.what());
    }
}

template<typename T, typename Key>
std::optional<T> parse_optional_field(nlohmann::json const & json, Key const & key)
{
    return parse_optional_field<T>(json, to_string(key));
}

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key, T def)
{
    auto field = parse_optional_field<T>(json, key);
    return field ? std::move(*field) : def;
}

template<typename T>
T parse_field(nlohmann::json const & json, char const * const key)
{
    using std::operator""s;

    auto field = parse_optional_field<T>(json, key);
    if (!field)
        throw std::invalid_argument("Field: '"s + key + "' not present in JSON: " + json.dump());
    return std::move(*field);
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

inline void to_json(color const & c, nlohmann::json & out)
{
    std::stringstream stream;
    stream
        << "#"
        << std::setfill('0') << std::setw(sizeof(rgba_t) * 2)
        << std::hex << c.rgba();
    out = stream.str();
}

inline void from_json(nlohmann::json const & json, color & out)
{
    using std::operator""s;

    if (!json.is_string())
        throw std::invalid_argument("Expected JSON string for color got: "s + json.type_name());

    std::string color = json;
    out = color == "random"
        ? random_color()
        : from_string(color);
}

} // End of namespace
