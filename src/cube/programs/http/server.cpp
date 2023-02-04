#include <cube/programs/http/server.hpp>
#include <cube/programs/http/listener.hpp>
#include <cube/programs/http/session.hpp>
#include <cube/core/logging.hpp>

using namespace cube::core;

namespace cube::programs::http
{

server::server(engine_context & context, net::ip::address interface, unsigned short port) :
    context_(context),
    interface_(interface),
    port_(port)
{ }

server::server(server && other) :
    context_(other.context_),
    listener_(std::move(other.listener_)),
    sessions_(std::move(other.sessions_)),
    interface_(std::move(other.interface_)),
    port_(other.port_),
    session_id_(other.session_id_)
{ }

void server::run(http_request_handler_t request_handler)
{
    listener_ = listener::create(
        context_,
        tcp::endpoint{interface_, port_},
        [this, h = std::move(request_handler)](auto socket)
            {
                auto const id = session_id_++;
                auto session = session::create(std::move(socket), context_.event_poller, h,
                    [this, id]() { sessions_.erase(id); });
                session->run();
                sessions_[id] = std::move(session);
            });
    listener_->run();

    std::string url_builder = "http://";
    url_builder += interface_.to_string();
    url_builder += ":";
    url_builder += std::to_string(port_);

    LOG_INF("Started HTTP server",
        LOG_ARG("interface", interface_.to_string()),
        LOG_ARG("port", port_),
        LOG_ARG("url", url_builder));
}

} // End of namespace
