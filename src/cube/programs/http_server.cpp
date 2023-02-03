#include <cube/programs/http/server.hpp>
#include <cube/programs/http/router.hpp>
#include <cube/programs/program.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/logging.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <driver/graphics_device.hpp>
#include <boost/beast/version.hpp>

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

    router->add_route(
        {
            /* curl --header "Content-Type: application/json" \
                    -X POST --data '{"animation":"fireworks"}' \
                    http://localhost:8080/api/animation
            */
            "/api/animation",
            [&engine](auto req)
            {
                if (req.method() == verb::get)
                    return response::bad_request("Yet to be implemented", req);

                nlohmann::json body;
                try {
                    body = nlohmann::json::parse(req.body());
                }  catch (std::exception const & ex) {
                    return response::bad_request(ex.what(), req);
                }

                auto animation = load_animation(body, engine.context());
                if (!animation)
                    return response::bad_request(animation.error().what, req);

                auto const & [_, x] = *animation;
                engine.load(std::static_pointer_cast<cube::core::animation>(x));

                return response::ok(req);
            },
            {verb::get, verb::post},
            mime_type::application_json
        });

    server.run(std::bind(&router::operator(), router, std::placeholders::_1));
    engine.run();

    return EXIT_SUCCESS;
}

program const program_http_server
{
    "http-server",
    "run the LED cube's engine and HTTP server",
    handle_http_server,
    []()
    {
        for (auto && handler : sigint_handlers)
            handler();
    }
};

} // End of namespace
