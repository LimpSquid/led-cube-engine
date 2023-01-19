#pragma once

#include <cube/gfx/json_utils.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/color.hpp>
#include <unordered_map>
#include <variant>

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

enum class animation_trait : unsigned int
{
    none = 0x0000,
    transition = 0x0001,
    all = UINT_MAX,
};

inline animation_trait operator|(animation_trait const lhs, animation_trait const rhs)
{
    using type = std::underlying_type_t<animation_trait>;
    return static_cast<animation_trait>(type(lhs) | type(rhs));
}

inline bool is_set(animation_trait const traits, animation_trait const flags)
{
    using type = std::underlying_type_t<animation_trait>;
    return (type(traits) & type(flags)) == type(flags);
}

class configurable_animation :
    public core::animation
{
public:
    std::string get_label() const;
    std::chrono::milliseconds get_duration() const;
    std::chrono::milliseconds get_transition_time() const; // returns zero if animation_trait::transition is not supported by the animation

    void load_properties(nlohmann::json const & json);
    nlohmann::json dump_properties() const;

protected:
    configurable_animation(core::engine_context & context);

    template<typename T>
    void write_property(std::string label, T value)
    {
        properties_[label] = property_value_t{std::move(value)};
    }

    template<typename T>
    T read_property(std::string label) const
    {
        using std::operator""s;

        property_value_t value = read_property_value(label);

        try {
            return std::get<T>(std::move(value));
        } catch(std::bad_variant_access const & ex) {
            throw std::invalid_argument("Property type mismatch for property: "s + label);
        }
    }

    property_value_t read_property_value(std::string label) const
    {
        using std::operator""s;

        auto const search_props = properties_.find(label);
        if (search_props != properties_.end())
            return search_props->second;

        auto const defaults = default_properties();
        auto const search_defaults = defaults.find(label);
        if (search_defaults  == defaults.end())
            throw std::invalid_argument("Unable to read property: "s + label);
        return search_defaults->second;
    }

private:
    virtual animation_trait traits() const;
    virtual std::unordered_map<std::string, property_value_t> extra_properties() const;

    std::unordered_map<std::string, property_value_t> default_properties() const;
    std::unordered_map<std::string, property_value_t> base_properties() const;

    std::unordered_map<std::string, property_value_t> properties_;
};

} // end of namespace
