#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/json_utils.hpp>

using namespace cube::core;
using namespace cube::gfx;
using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr std::string_view default_label{""};
constexpr milliseconds default_duration{15000};
constexpr milliseconds default_transition_time{2000};
constexpr double no_motion_blur{0};

} // End of namespace


namespace cube::gfx
{

std::string configurable_animation::get_label() const
{
    return read_property<std::string>("label");
}

milliseconds configurable_animation::get_duration() const
{
    return read_property<milliseconds>("duration_ms");
}

milliseconds configurable_animation::get_transition_time() const
{
    return is_set(traits(), animation_trait::transition)
        ? read_property<milliseconds>("transition_time_ms")
        : milliseconds{};
}

double configurable_animation::get_motion_blur() const
{
    return read_property<double>("motion_blur");
}

void configurable_animation::load_properties(nlohmann::json const & json)
{
    if (!json.is_object())
        throw std::runtime_error("Expected JSON object got: "s + json.type_name());

    auto defaults = default_properties();
    for (auto & [label, def] : defaults) {
        auto value = std::visit([&](auto & def) -> property_value_t {
            return {parse_field(json, label.c_str(), std::move(def))};
        }, def);

        write_property(std::move(label), std::move(value));
    }
}

nlohmann::json configurable_animation::dump_properties() const
{
    nlohmann::json json = nlohmann::json::object();

    auto defaults = default_properties();
    for (auto & [label, _] : defaults) {
        auto value = read_property_value(label);

        std::visit([&](auto & value) {
            json.emplace(make_field(label.c_str(), value));
        }, value);
    }

    return json;
}

configurable_animation::configurable_animation(engine_context & context) :
    animation(context)
{ }

std::optional<double> configurable_animation::motion_blur() const
{
    auto const blur = get_motion_blur();
    return blur == no_motion_blur
        ? std::optional<double>{}
        : std::optional{blur};
}

animation_trait configurable_animation::traits() const
{
    return animation_trait::none;
}

std::unordered_map<std::string, property_value_t> configurable_animation::extra_properties() const
{
    return {};
}

std::unordered_map<std::string, property_value_t> configurable_animation::default_properties() const
{
    auto x = extra_properties();
    auto b = base_properties();
    x.insert(b.begin(), b.end()); // Extra precedes base

    return x;
}

std::unordered_map<std::string, property_value_t> configurable_animation::base_properties() const
{
    std::unordered_map<std::string, property_value_t> properties = {
        {"label", std::string(default_label)},
        {"duration_ms", default_duration},
        {"motion_blur", no_motion_blur},
    };

    if (is_set(traits(), animation_trait::transition))
        properties["transition_time_ms"] = default_transition_time;

    return properties;
}

} // End of namespace
