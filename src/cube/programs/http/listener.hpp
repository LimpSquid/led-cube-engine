#pragma once

#include <cube/core/events.hpp>
#include <boost/beast/core.hpp>

namespace cube::core { class engine_context; }
namespace cube::programs::http
{

namespace net = boost::asio;
using tcp = net::ip::tcp;

class listener :
    public std::enable_shared_from_this<listener>
{
public:
    using on_connection_handler_t = std::function<void(tcp::socket)>;

    template<typename ... A>
    static std::shared_ptr<listener> create(A && ... args)
    {
        return std::shared_ptr<listener>(new listener(std::forward<A>(args) ...));
    }

    void run();

private:
    listener(core::engine_context & context, tcp::endpoint endpoint, on_connection_handler_t on_conn_handler);

    void do_accept();
    void on_accept(boost::beast::error_code ec, tcp::socket socket);

    tcp::acceptor acceptor_;
    std::optional<core::fd_event_notifier> read_poll_;
    on_connection_handler_t on_conn_handler_;
};

} // End of namespace
