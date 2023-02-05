#include <cube/programs/http/server.hpp>
#include <cube/programs/http/router.hpp>
#include <cube/programs/program.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/logging.hpp>
#include <cube/core/json_utils.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <driver/graphics_device.hpp>

using namespace cube::core;
using namespace cube::gfx;
using namespace cube::programs;
using namespace cube::programs::http;

namespace
{

std::vector<cube::programs::program_sigint_t> sigint_handlers;

render_engine<driver::graphics_device_t> & engine_instance()
{
    struct singleton
    {
        singleton()
        {
            sigint_handlers.push_back([&]() {
                LOG_INF("Stopping rendering engine and HTTP server");
                engine.stop();
            });
        }

        engine_context context;
        render_engine<driver::graphics_device_t> engine{context};
    };

    static singleton s;
    return s.engine;
}

namespace api
{

std::optional<std::string> current_animation;

void show_animation(animation_t animation)
{
    auto & engine = engine_instance();

    auto [name, x] = std::move(animation);
    engine.load(x);
    current_animation.emplace(std::move(name));
}

std::string current_animation_name()
{
    return current_animation ? *current_animation : "";
}

route const route_render =
{
    "/api/render",
    [](auto req)
    {
        if (req.method() == verb::get)
            return response::ok_text(current_animation_name(), req);

        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body());
        }  catch (std::exception const & ex) {
            return response::bad_request(ex.what(), req);
        }

        auto & engine = engine_instance();
        auto animation = load_animation(body, engine.context());
        if (!animation)
            return response::bad_request(animation.error().what, req);

        show_animation(std::move(*animation));
        return response::ok(req);
    },
    {verb::get, verb::post},
    mime_type::application_json
};

route const route_render_stop =
{
    "/api/render/stop",
    [](auto req)
    {
        show_animation({});
        return response::ok(req);
    },
    {verb::get, verb::post}
};

route const route_animation_info =
{
    "/api/animation/info",
    [](auto req)
    {
        std::string animation_name;
        try {
            nlohmann::json json = nlohmann::json::parse(req.body());
            animation_name = parse_field<std::string>(json, "animation");
        }  catch (std::exception const & ex) {
            return response::bad_request(ex.what(), req);
        }

        auto & engine = engine_instance();
        auto incubated = library::instance().incubate(animation_name, engine.context());
        if (!incubated)
            return response::bad_request(incubated.error().what, req);

        return response::ok_json((*incubated)->dump_properties().dump(-1), req);
    },
    {verb::post},
    mime_type::application_json
};

route const route_animation_list =
{
    "/api/animation/list",
    [](auto req)
    {
        auto & engine = engine_instance();
        auto & lib = library::instance();

        nlohmann::json json = nlohmann::json::array();

        for (auto const & animation_name : lib.available_animations()) {
            auto incubated = lib.incubate(animation_name, engine.context());
            if (!incubated) // Should never happen
                return response::internal_server_error(req);

            nlohmann::json item = nlohmann::json::object();
            item.emplace(make_field("animation", animation_name));
            item.emplace(make_field("properties", (*incubated)->dump_properties()));
            json.push_back(std::move(item));
        }

        return response::ok_json(json.dump(-1), req);
    },
    {verb::get}
};

} // End of namespace

int handle_http_server(int ac, char const * const av[])
{
    if (ac != 2) {
        std::cout
            << "Usage: led-cube-engine http-server <interface>:<port>\n\n"
            << "Examples:\n"
            << "  led-cube-engine http-server 0.0.0.0:8080\n";
        return EXIT_FAILURE;
    }

    auto & engine = engine_instance();
    auto server = make_server_from_string(engine.context(), av[1]);

    auto router = router::create();
    router->add_route(api::route_render)
           .add_route(api::route_render_stop)
           .add_route(api::route_animation_info)
           .add_route(api::route_animation_list);

    server.run(router->mux());
    engine.run();

    return EXIT_SUCCESS;
}

program const program_http_server
{
    "http-server",
    "run the LED cube's render engine and HTTP server",
    handle_http_server,
    []()
    {
        for (auto && handler : sigint_handlers)
            handler();
    }
};

} // End of namespace
