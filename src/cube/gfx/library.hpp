#pragma once

#include <cube/core/expected.hpp>
#include <3rdparty/nlohmann/json.hpp>
#include <boost/type_index.hpp>

namespace cube::core { class engine_context; }
namespace cube::gfx
{

class configurable_animation;
using animation_pointer_t = std::shared_ptr<configurable_animation>;
using animation_incubator_t = std::function<animation_pointer_t(core::engine_context &)>;
using animation_list_t = std::vector<std::pair<std::string, animation_pointer_t>>;

class library
{
public:
    static library & instance();

    std::vector<std::string> available_animations() const;
    core::expected_or_error<animation_pointer_t> incubate(std::string const & animation, core::engine_context & context) const;
    void publish_animation(std::string const & name, animation_incubator_t incubator);

private:
    library() = default;
    library(library &) = delete;
    library(library &&) = delete;

    std::unordered_map<std::string, animation_incubator_t> animations_;
};

template<typename T>
struct animation_publisher
{
    animation_publisher(std::string const & name)
    {
        library::instance().publish_animation(
            name,
            [](core::engine_context & context) { return std::make_shared<T>(context); }
        );
    }

    animation_publisher()
    {
        auto const trim_namespace = [](std::string s) {
            auto const colon = s.find_last_of(":");
            return colon == std::string::npos ? s : s.substr(colon + 1);
        };

        library::instance().publish_animation(
            trim_namespace(boost::typeindex::type_id<T>().pretty_name()),
            [](core::engine_context & context) { return std::make_shared<T>(context); }
        );
    }
};

core::expected_or_error<animation_list_t> load_animations(nlohmann::json const & json, core::engine_context & context);

}
