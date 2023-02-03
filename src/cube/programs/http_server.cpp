#include <cube/programs/http/server.hpp>
#include <cube/programs/program.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/logging.hpp>
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

http_response_t handle_request(http_request_t req)
{
    using namespace boost::beast::http;

    auto const bad_request = [&req](std::string why)
        {
            http_response_t res{status::bad_request, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::move(why);
            res.prepare_payload();
            return res;
        };

    auto const not_found = [&req]()
        {
            http_response_t res{status::not_found, req.version()};
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "";
            res.prepare_payload();
            return res;
        };

    // Make sure we can handle the method
    if( req.method() != verb::get &&
        req.method() != verb::head)
        return bad_request("Unknown HTTP-method");

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().size() <= 2 ||
        req.target().find("..") != std::string_view::npos)
        return bad_request("Illegal request-target");

    auto target = req.target();
    target.remove_prefix(1);
    std::string const animation_name = std::string(target);

    auto & engine = engine_instance();
    auto animation = library::instance().incubate(animation_name, engine.context());

    if (!animation) {
        LOG_INF("Animation not found!", LOG_ARG("name", animation_name));
        return not_found();
    }

    engine.load(std::static_pointer_cast<cube::core::animation>(*animation));

    http_response_t res{status::ok, req.version()};
    res.set(field::server, BOOST_BEAST_VERSION_STRING);
    res.set(field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "";
    res.prepare_payload();
    return res;
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

    server.run(handle_request);
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
