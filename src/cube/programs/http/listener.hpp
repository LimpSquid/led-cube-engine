#pragma once

#include <cube/core/utils.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <boost/beast/core.hpp>

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

    void run()
    {
        do_accept();
    }

private:
    listener(core::engine_context & context, tcp::endpoint endpoint, on_connection_handler_t on_conn_handler) :
        acceptor_(context.io_context),
        on_conn_handler_(on_conn_handler)
    {
        boost::beast::error_code ec;

        auto const fail = [&](std::string what) {
            throw std::runtime_error(what + ": " + ec.message());
        };

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) fail("open");

        // Allow to reuse address if it's in the TIME_WAIT state
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) fail("set option");

        acceptor_.bind(endpoint, ec);
        if (ec) fail("bind");

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) fail("listen");

        read_poll_.emplace(context.event_poller, acceptor_.native_handle(), core::fd_event_notifier::read);
    }

    void do_accept()
    {
        acceptor_.async_accept(core::scoped_handler(
            std::bind(&listener::on_accept,
                this,
                std::placeholders::_1,
                std::placeholders::_2),
            *this));
    }

    void on_accept(boost::beast::error_code ec, tcp::socket socket)
    {
        if (ec == net::error::operation_aborted) return;
        if (ec)
            LOG_WRN("Failed to accept TCP connection", LOG_ARG("error", ec.message()));
        else {
            LOG_DBG("Accepted TCP connection");
            on_conn_handler_(std::move(socket));
        }

        do_accept();
    }

    tcp::acceptor acceptor_;
    std::optional<core::fd_event_notifier> read_poll_;
    on_connection_handler_t on_conn_handler_;
};

} // End of namespace
