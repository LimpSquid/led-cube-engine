#pragma once

#include <cube/gfx/json_util.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/enum.hpp>
#include <cube/core/color.hpp>
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

class configurable_animation :
    public core::animation
{
public:
    std::string get_label() const;
    std::chrono::milliseconds get_duration() const;

    void load_properties(nlohmann::json const & json);
    nlohmann::json dump_properties() const;

protected:
    using property_label_t = int;
    using property_pair_t = std::pair<property_label_t, property_value_t>;

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

    template<typename T, typename L>
    T read_property(L label, T def = {}) const
    {
        using std::operator""s;
        auto const search = properties_.find(label);
        if (search == properties_.end())
            return def;

        try {
            return std::get<T>(search->second);
        } catch(std::bad_variant_access const & ex) {
            if constexpr(std::is_enum_v<L>) // If the enum was used we can give a more detailed description
                throw std::invalid_argument("Property type mismatch for property: "s + to_string(label));
            else
                throw std::invalid_argument("Property type mismatch");
        }
    }

    void write_properties(std::vector<property_pair_t> const & properties);

    template<typename T, typename L>
    nlohmann::json to_json(L label, T def = {}) const { return core::make_field(label, read_property(label, def)); }
    template<typename T, typename L>
    property_pair_t from_json(nlohmann::json const & json, L label) const { return {label, core::parse_field<T>(json, label)}; }
    template<typename T, typename L>
    property_pair_t from_json(nlohmann::json const & json, L label, T def) const { return {label, core::parse_field(json, label, def)}; }

private:
    virtual nlohmann::json properties_to_json() const;
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const;

    std::unordered_map<property_label_t, property_value_t> properties_;
};

} // end of namespace

#undef PROPERTY_ENUM
#define PROPERTY_ENUM(...) ENUM(property, property_label_t, 255, __VA_ARGS__)
