#pragma once

#include <cube/gfx/property_value.hpp>
#include <cube/core/animation.hpp>
#include <cube/core/enum.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <unordered_map>

#define PROPERTY_ENUM(...) \
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

    PROPERTY_ENUM
    (
        animation_label,
    )

    template<typename T>
    void write_property(property_label_type label, T value)
    {
        properties_[label] = property_value{std::move(value)};
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

    void write_properties(std::vector<property_pair> const & properties);
    void load_properties(nlohmann::json const & json);
    nlohmann::json dump_properties() const;

protected:
    configurable_animation(core::engine_context & context);

private:
    virtual nlohmann::json properties_to_json() const = 0;
    virtual std::vector<property_pair> properties_from_json(nlohmann::json const & json) const = 0;

    std::unordered_map<property_label_type, property_value> properties_;
};

} // end of namespace

#undef PROPERTY_ENUM
#define PROPERTY_ENUM(...) ENUM(property, property_label_type, 255, __VA_ARGS__)
