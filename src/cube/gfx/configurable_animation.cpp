#include <cube/gfx/configurable_animation.hpp>

using namespace cube::core;
using namespace std::chrono;
using std::operator""s;

namespace cube::gfx
{

void configurable_animation::write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties)
{
    for (auto const & property : properties)
        write_property(property.first, property.second);
}

void configurable_animation::load_properties(nlohmann::json const & json)
{
    if (!json.is_object())
        throw std::runtime_error("Expected JSON object got: "s + json.type_name());

    write_properties(parse(json));
}

configurable_animation::configurable_animation(engine_context & context, char const * const name) :
    animation(context, name)
{ }

std::vector<configurable_animation::property_pair> configurable_animation::parse(nlohmann::json const &) const
{
    throw std::runtime_error("JSON properties not supported for animation: "s + name());
}

} // End of namespace
