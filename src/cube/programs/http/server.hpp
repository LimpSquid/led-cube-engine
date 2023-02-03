#pragma once

#include <cube/programs/http/listener.hpp>
#include <cube/programs/http/session.hpp>
#include <cube/core/engine_context.hpp>

namespace cube::programs::http
{

namespace net = boost::asio;

class server
{
public:
    server(core::engine_context & context, net::ip::address interface, unsigned short port) :
        context_(context),
        interface_(interface),
        port_(port)
    { }

    server(server && other) :
        context_(other.context_),
        listener_(std::move(other.listener_)),
        sessions_(std::move(other.sessions_)),
        interface_(std::move(other.interface_)),
        port_(other.port_),
        session_id_(other.session_id_)
    { }

    void run(session::request_handler_t request_handler)
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

        LOG_INF("Started HTTP server.",
            LOG_ARG("interface", interface_.to_string()),
            LOG_ARG("port", port_));
    }

private:
    server(server const &) = delete;

    core::engine_context & context_;
    std::shared_ptr<listener> listener_;
    std::unordered_map<int, std::shared_ptr<session>> sessions_;

    net::ip::address interface_;
    unsigned short port_;
    int session_id_{0};
};


inline server make_server_from_string(core::engine_context & context, std::string const & address)
{
    using std::operator""s;

    auto const colon = address.find(':');
    if (colon == std::string::npos)
        throw std::runtime_error("address '"s + address + "' should contain a colon, i.e. 'interface:port'");

    auto const interface = net::ip::make_address(address.substr(0, colon).c_str());
    unsigned short port;
    try {
        port = static_cast<unsigned short>(std::stoul(address.substr(colon + 1).c_str(), nullptr, 10));
    } catch(...) {
        throw std::runtime_error("Unable to parse port");
    }

    return server{context, std::move(interface), port};
}

}
