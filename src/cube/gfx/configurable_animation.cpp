#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/json_utils.hpp>

using namespace cube::core;
using namespace cube::gfx;
using namespace std::chrono;
using std::operator""s;

namespace
{

constexpr std::string_view default_label{""};
constexpr milliseconds default_duration{15000};

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

void configurable_animation::load_properties(nlohmann::json const & json)
{
    if (!json.is_object())
        throw std::runtime_error("Expected JSON object got: "s + json.type_name());

    auto defaults = default_properties();
    for (auto & [label, def] : defaults) {
        auto value = std::visit([&](auto & def) -> property_value_t {
            return {parse_field(json, label.c_str(), std::move(def))};
        }, def);

        write_property(label, std::move(value));
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

std::unordered_map<std::string, property_value_t> configurable_animation::default_properties() const
{
    auto x = extra_properties();
    auto b = base_properties();
    x.insert(b.begin(), b.end()); // Base overwrites extra

    return x;
}

std::unordered_map<std::string, property_value_t> configurable_animation::base_properties() const
{
    return {
        { "label", std::string(default_label) },
        { "duration_ms", default_duration },
    };
}

std::unordered_map<std::string, property_value_t> configurable_animation::extra_properties() const
{
    return {};
}

} // End of namespace
