#include <cube/gfx/configurable_animation.hpp>

using namespace cube::core;
using namespace std::chrono;

namespace cube::gfx
{

void configurable_animation::write_properties(std::vector<std::pair<property_label_type, property_value>> const & properties)
{
    for (auto const & property : properties)
        write_property(property.first, property.second);
}

configurable_animation::configurable_animation(engine_context & context) :
    context_(context)
{ }

engine_context & configurable_animation::context()
{
    return context_;
}

} // End of namespace
