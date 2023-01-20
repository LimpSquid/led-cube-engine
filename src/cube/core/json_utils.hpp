#pragma once

#include <cube/core/color.hpp>
#include <cube/core/expression.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/type_index.hpp>
#include <optional>

namespace cube::core
{
template<typename T, typename = void>
struct is_json_convertible : std::false_type {};
template<typename T>
struct is_json_convertible<T,
    std::void_t<
        decltype(to_json(std::declval<typename T::value_type>(), std::declval<nlohmann::json &>())),
        decltype(from_json(std::declval<nlohmann::json>(), std::declval<typename T::value_type &>()))>
    > : std::true_type {};
template<typename T>
constexpr bool is_json_convertible_v = is_json_convertible<T>::value;

template<typename T, typename = void>
struct is_json_iterable : std::false_type {};
template<typename T>
struct is_json_iterable<T,
    std::void_t<
        decltype(std::begin(std::declval<T>())),
        decltype(std::end(std::declval<T>()))>
    > : is_json_convertible<T> {};
template<typename T>
constexpr bool is_json_iterable_v = is_json_iterable<T>::value;

template<typename T, typename = void>
struct is_json_insertable : std::false_type {};
template<typename T>
struct is_json_insertable<T,
    std::void_t<
        decltype(std::end(std::declval<T>())),
        decltype(std::declval<T>().insert(std::end(std::declval<T>()), std::declval<typename T::value_type>()))>
    > : is_json_convertible<T> {};
template<typename T>
constexpr bool is_json_insertable_v = is_json_insertable<T>::value;

inline void to_json(nlohmann::json const & json, nlohmann::json & out) { out = json; }
inline void from_json(nlohmann::json const & json, nlohmann::json & out) { out = json; }

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
    if constexpr (is_json_insertable_v<T> && !std::is_same_v<T, std::string>) {
        using std::operator""s;

        if (!json.is_array())
            throw std::invalid_argument("Expected JSON array got: "s + json.type_name());

        typename T::value_type element_out;
        for (auto const & element : json) {
            from_json(element, element_out);
            out.insert(std::end(out), std::move(element_out));
        }
    } else {
#ifdef EVAL_EXPRESSION
        if constexpr (std::is_arithmetic_v<T>) {
            if (json.is_string()) {
                if constexpr (std::is_integral_v<T>) {
                    using std::operator""s;
                    double const value = std::round(evald(json));
                    if (value > static_cast<double>(std::numeric_limits<T>::max()) ||
                        value < static_cast<double>(std::numeric_limits<T>::min()))
                        throw std::overflow_error("Expression eval result overflowed for integral type: "s +
                            boost::typeindex::type_id<T>().pretty_name());
                    out = static_cast<T>(value);
                    return;
                } else if constexpr (std::is_floating_point_v<T>) {
                    out = eval<T>(json);
                    return;
                }
            }
        }
#endif
        out = T(json);
    }
}

template<class Rep, class Period>
void to_json(std::chrono::duration<Rep, Period> const & duration, nlohmann::json & out)
{
    to_json(duration.count(), out);
}

template<class Rep, class Period>
void from_json(nlohmann::json const & json, std::chrono::duration<Rep, Period> & out)
{
    Rep ticks;
    from_json(json, ticks);
    out = std::chrono::duration<Rep, Period>{ticks};
}

template<typename T>
std::optional<T> parse_optional_field(nlohmann::json const & json, char const * const key)
{
    using std::operator""s;

    auto const i = json.find(key);
    if (i == json.end())
        return {};

    try {
        T out;
        from_json(*i, out);
        return {std::move(out)};
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to parse field: '"s + key
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
        nlohmann::json out;
        to_json(value, out);
        return {key, std::move(out)};
    } catch(std::exception const & ex) {
        throw std::invalid_argument("Unable to make field: '"s + key
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
    out = color_to_string(c);
}

inline void from_json(nlohmann::json const & json, color & out)
{
    using std::operator""s;

    if (!json.is_string())
        throw std::invalid_argument("Expected JSON string got: "s + json.type_name());

    std::string color = json;
    out = color == "random"
        ? random_color()
        : color_from_string(color);
}

} // End of namespace
