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
    return read_property(label, std::string(default_label));
}

milliseconds configurable_animation::get_duration() const
{
    return read_property(duration_ms, default_duration);
}

void configurable_animation::load_properties(nlohmann::json const & json)
{
    if (!json.is_object())
        throw std::runtime_error("Expected JSON object got: "s + json.type_name());

    auto properties = properties_from_json(json);
    if (!properties)
        throw std::runtime_error("Failed to load properties: " + properties.error().what);

    properties->push_back(make_property(json, label, std::string(default_label)));
    properties->push_back(make_property(json, duration_ms, default_duration));
    write_properties(*properties);
}

nlohmann::json configurable_animation::dump_properties() const
{
    nlohmann::json json =
    {
        make_json(label, std::string(default_label)),
        make_json(duration_ms, default_duration),
    };

    auto other = properties_to_json();
    if (!other)
        throw std::runtime_error("Failed to dump properties: " + other.error().what);
    if (!other->empty()) {
        assert(other->is_object());
        json.update(*other); // If other is no object this will throw an exception
    }
    return json;
}

configurable_animation::configurable_animation(engine_context & context) :
    animation(context)
{ }

void configurable_animation::write_properties(property_pairs_t const & properties)
{
    for (auto const & property : properties)
        write_property(property.first, property.second);
}

json_or_error_t configurable_animation::properties_to_json() const
{
    return nlohmann::json{};
}

property_pairs_or_error_t configurable_animation::properties_from_json(nlohmann::json const &) const
{
    return property_pairs_t{};
}

} // End of namespace
