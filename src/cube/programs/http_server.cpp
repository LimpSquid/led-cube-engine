#include <cube/programs/http_server/listener.hpp>
#include <cube/programs/http_server/session.hpp>
#include <cube/core/engine.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <cube/programs/program.hpp>
#include <cube/gfx/library.hpp>
#include <cube/gfx/configurable_animation.hpp>
#include <driver/graphics_device.hpp>

using namespace cube::core;
using namespace cube::gfx;
using namespace cube::programs;
using namespace cube::programs::http_server;

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
    auto const bad_request = [&req](std::string why)
        {
            http_response_t res{http::status::bad_request, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::move(why);
            res.prepare_payload();
            return res;
        };

    auto const not_found = [&req]()
        {
            http_response_t res{http::status::not_found, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "";
            res.prepare_payload();
            return res;
        };

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head)
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

    http_response_t res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "";
    res.prepare_payload();
    return res;
}

class http_server
{
public:
    http_server(net::ip::address interface, unsigned short port) :
        interface_(interface),
        port_(port)
    { }

    void run()
    {
        auto & engine = engine_instance();

        listener_ = listener::create(
            engine.context(),
            tcp::endpoint{interface_, port_},
            [this, &ctx = engine.context()](auto socket)
                {
                    auto const id = session_id_++;
                    auto session = session::create(std::move(socket), ctx.event_poller, handle_request,
                        [this, id]() { sessions_.erase(id); });
                    session->run();
                    sessions_[id] = std::move(session);
                });
        listener_->run();
    }

private:
    std::shared_ptr<listener> listener_;
    std::unordered_map<int, std::shared_ptr<session>> sessions_;

    net::ip::address interface_;
    unsigned short port_;
    int session_id_{0};
};

int handle_http_server(int ac, char const * const av[])
{
    if (ac != 3)
    {
        std::cerr <<
            "Usage: ./led-cube-engine http-server <address> <port>\n" <<
            "Example:\n" <<
            "     ./led-cube-engine http-server 0.0.0.0 8080\n";
        return EXIT_FAILURE;
    }

    auto & engine = engine_instance();
    auto const address = net::ip::make_address(av[1]);
    auto const port = static_cast<unsigned short>(std::atoi(av[2]));

    http_server server{address, port};

    server.run();
    engine.run();

    return EXIT_SUCCESS;
}

program const program_http_server
{
    "http-server",
    "run the LED cube's engine HTTP server.",
    handle_http_server,
    []()
    {
        for (auto && handler : sigint_handlers)
            handler();
    }
};

} // End of namespace
