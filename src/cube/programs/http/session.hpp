#pragma once

#include <cube/core/engine_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

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

    void run();

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
        finished_handler_t finished_handler = {});

    void do_read(std::shared_ptr<life_sign> life_sign);
    void on_read(std::shared_ptr<life_sign> life_sign, boost::beast::error_code ec);
    void on_write(std::shared_ptr<life_sign> life_sign, bool close, boost::beast::error_code ec);
    void do_close(std::shared_ptr<life_sign>);

    http_request_t request_;
    http_response_t response_;

    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    core::fd_event_notifier rw_poll_;

    request_handler_t request_handler_;
    finished_handler_t finished_handler_;
};

} // End of namespace
