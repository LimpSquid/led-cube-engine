#include <cube/gfx/library.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <cube/core/json_utils.hpp>
#include <cube/core/logging.hpp>
#include <algorithm>

using namespace cube::core;
using std::operator""s;

namespace cube::gfx
{

library & library::instance()
{
    static library instance;
    return instance;
}

std::vector<std::string> library::available_animations() const
{
    std::vector<std::string> result(animations_.size());
    std::transform(animations_.begin(), animations_.end(),
        result.begin(), [](auto const & pair) { return pair.first; });
    return result;
}

void library::publish_animation(std::string const & name, animation_incubator_t incubator)
{
    if (animations_.find(name) != animations_.cend())
        throw std::runtime_error("Failed to publish animation '" + name + "', name already exists");
    animations_[name] = std::move(incubator);
}

expected_or_error<animation_pointer_t> library::incubate(std::string const & animation, engine_context & context) const
{
    auto const search = animations_.find(animation);

    if (search == animations_.end())
        return unexpected_error{"Animation library does not contain: "s + animation};

    auto incubated = search->second(context);
    if (!incubated)
        return unexpected_error{"Unable to incubate animation: "s + animation};
    return {std::move(incubated)};
}

expected_or_error<animation_t> load_animation(nlohmann::json const & json, engine_context & context)
{
    if (!json.is_object())
        return unexpected_error{"Expected JSON object got: "s + json.type_name()};

    library & lib = library::instance();
    animation_t result;

    try {
        auto const animation = parse_field<std::string>(json, "animation");
        auto const properties = parse_field(json, "properties", nlohmann::json({})); // Or default properties

        auto incubated = lib.incubate(animation, context);
        if (!incubated)
            return incubated.error();

        (*incubated)->load_properties(properties);
        result = {animation, std::move(*incubated)};

        LOG_DBG("Loaded animation",
            LOG_ARG("animation", animation),
            LOG_ARG("properties", properties.dump(-1)));
    } catch (std::exception const & ex) {
        return unexpected_error{ex.what()};
    }

    return result;
}

expected_or_error<animation_list_t> load_animations(nlohmann::json const & json, engine_context & context)
{
    if (!json.is_array())
        return unexpected_error{"Expected JSON array got: "s + json.type_name()};

    animation_list_t result;

    try {
        for (auto const & element : json) {
            if (!parse_field(element, "enabled", true))
                continue;

            auto animation = load_animation(element, context);
            if (!animation)
                return animation.error();

            result.push_back(std::move(*animation));
        }
    } catch (std::exception const & ex) {
        return unexpected_error{ex.what()};
    }

    return {std::move(result)};
}

}
