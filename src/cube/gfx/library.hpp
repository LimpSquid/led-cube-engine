#pragma once

#include <cube/core/expected.hpp>
#include <3rdparty/nlohmann/json.hpp>

namespace cube::core { class engine_context; }
namespace cube::gfx
{

class configurable_animation;
using animation_pointer_t = std::unique_ptr<configurable_animation>;
using animation_incubator_t = std::function<animation_pointer_t(core::engine_context &)>;

class library
{
public:
    static library & instance();

    std::vector<std::string> available_animations() const;
    core::expected_or_error<animation_pointer_t> incubate(std::string const & animation, core::engine_context & context) const;
    void publish_animation(std::string const & name, animation_incubator_t incubator);

private:
    library() = default;

    std::unordered_map<std::string, animation_incubator_t> animations_;
};

template<typename Animation>
struct animation_publisher
{
    animation_publisher(char const * const animation_name)
    {
        library::instance().publish_animation(animation_name, [](core::engine_context & context)
            { return std::make_unique<Animation>(context); });
    }
};

core::expected_or_error<std::vector<animation_pointer_t>> load_animations(nlohmann::json const & json, core::engine_context & context);

}
