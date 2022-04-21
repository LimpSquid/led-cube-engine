#pragma once

#include <cube/gfx/json_utils.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/enum.hpp>
#include <cube/core/color.hpp>
#include <cube/core/expected.hpp>
#include <unordered_map>
#include <variant>

#define PROPERTY_ENUM(...) \
    static_assert(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__) < 255); \
    ENUM(property, property_label_t, 0, __VA_ARGS__)

namespace cube::gfx
{

using property_value_t = std::variant<
    int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
    float, double, long double,
    std::string,
    std::chrono::nanoseconds, std::chrono::milliseconds, std::chrono::seconds,
    core::color, std::vector<core::color>,
    gradient, gradient_stop>;
using property_label_t = int;
using property_pair_t = std::pair<property_label_t, property_value_t>;
using property_pairs_t = std::vector<property_pair_t>;
using json_or_error_t = core::expected_or_error<nlohmann::json>;
using property_pairs_or_error_t = core::expected_or_error<property_pairs_t>;

template<typename T>
property_pair_t make_property(property_label_t label, T value)
{
    return std::make_pair(label, property_value_t{std::move(value)});
}

template<typename T, typename E>
property_pair_t make_property(nlohmann::json const & json, E label)
{
    return make_property(label, core::parse_field<T>(json, label));
}

template<typename T, typename E>
property_pair_t make_property(nlohmann::json const & json, E label, T def)
{
    return make_property(label, core::parse_field(json, label, std::move(def)));
}

class configurable_animation :
    public core::animation
{
public:
    std::string get_label() const;
    std::chrono::milliseconds get_duration() const;

    void load_properties(nlohmann::json const & json);
    nlohmann::json dump_properties() const;

protected:
    PROPERTY_ENUM
    (
        label,
        duration_ms,
    )

    configurable_animation(core::engine_context & context);

    template<typename T>
    void write_property(property_label_t label, T value)
    {
        properties_[label] = property_value_t{std::move(value)};
    }

    template<typename T, typename E>
    T read_property(E label, T def = {}) const
    {
        using std::operator""s;

        auto const search = properties_.find(label);
        if (search == properties_.end())
            return def;

        try {
            return std::get<T>(search->second);
        } catch(std::bad_variant_access const & ex) {
            if constexpr(std::is_enum_v<E>) // If the enum was used we can give a more detailed description
                throw std::invalid_argument("Property type mismatch for property: "s + to_string(label));
            else
                throw std::invalid_argument("Property type mismatch");
        }
    }

    void write_properties(property_pairs_t const & properties);

    template<typename T, typename E>
    nlohmann::json make_json(E label, T def = {}) const
    {
        return core::make_field(label, read_property(label, def));
    }

private:
    virtual json_or_error_t properties_to_json() const;
    virtual property_pairs_or_error_t properties_from_json(nlohmann::json const & json) const;

    std::unordered_map<property_label_t, property_value_t> properties_;
};

} // end of namespace

#undef PROPERTY_ENUM
#define PROPERTY_ENUM(...) ENUM(property, property_label_t, 255, __VA_ARGS__)
