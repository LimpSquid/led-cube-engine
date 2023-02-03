#pragma once

#include <cube/core/utils.hpp>
#include <cube/core/engine_context.hpp>
#include <cube/core/logging.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace cube::programs::http
{

namespace net = boost::asio;
namespace http = boost::beast::http;
using tcp = net::ip::tcp;

using http_request_t = http::request<http::string_body>;
using http_response_t = http::response<http::string_body>;

class session :
    public std::enable_shared_from_this<session>
{
public:
    using finished_handler_t = std::function<void()>;
    using request_handler_t = std::function<http_response_t(http_request_t const &)>;

    template<typename ... A>
    static std::shared_ptr<session> create(A && ... args)
    {
        return std::shared_ptr<session>(new session(std::forward<A>(args) ...));
    }

    void run()
    {
        do_read(std::make_shared<life_sign>(*this));
    }

private:
    struct life_sign
    {
        life_sign(session const & self) :
            self_(self.weak_from_this())
        { }

        ~life_sign()
        {
            if (auto const self = self_.lock()) {
                if (self->finished_handler_)
                    self->finished_handler_();
            }
        }

        std::weak_ptr<session const> self_;
    };

    session(tcp::socket && socket, core::event_poller_t & poller,
        request_handler_t request_handler,
        finished_handler_t finished_handler = {}) :
        stream_(std::move(socket)),
        buffer_(8 * 1024),
        rw_poll_(poller, stream_.socket().native_handle()),
        request_handler_(std::move(request_handler)),
        finished_handler_(std::move(finished_handler))
    { }

    void do_read(std::shared_ptr<life_sign> life_sign)
    {
        using std::operator""s;

        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        request_ = {};

        stream_.expires_after(15s);

        // Read the request
        rw_poll_.set_events(core::fd_event_notifier::read);
        http::async_read(stream_, buffer_, request_,
            core::scoped_handler(std::bind(
                &session::on_read,
                this,
                life_sign,
                std::placeholders::_1),
            *this));
    }

    void on_read(std::shared_ptr<life_sign> life_sign, boost::beast::error_code ec)
    {
        // Client closed the connection
        if (ec == http::error::end_of_stream)
            return do_close(life_sign);
        if (ec == boost::beast::error::timeout)
            return; // Ignore timeouts
        if (ec) {
            LOG_WRN("HTTP session failed to read", LOG_ARG("error", ec.message()));
            return;
        }

        response_ = request_handler_(std::move(request_));

        // Write the response
        rw_poll_.set_events(core::fd_event_notifier::write);
        http::async_write(stream_, response_,
            core::scoped_handler(std::bind(
                &session::on_write,
                this,
                life_sign,
                response_.need_eof(),
                std::placeholders::_1),
            *this));
    }

    void on_write(std::shared_ptr<life_sign> life_sign, bool close, boost::beast::error_code ec)
    {
        if (ec) {
            LOG_WRN("HTTP session failed to write", LOG_ARG("error", ec.message()));
            return;
        }

        if (close)
            return do_close(life_sign);

        do_read(life_sign);
    }

    void do_close(std::shared_ptr<life_sign>)
    {
        boost::beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // ignore the error
    }

    http_request_t request_;
    http_response_t response_;

    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    core::fd_event_notifier rw_poll_;

    request_handler_t request_handler_;
    finished_handler_t finished_handler_;
};

}
