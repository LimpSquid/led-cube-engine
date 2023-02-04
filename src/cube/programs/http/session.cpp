#include <cube/programs/http/session.hpp>
#include <cube/core/utils.hpp>
#include <cube/core/events.hpp>
#include <cube/core/logging.hpp>

using namespace cube::core;
namespace beast = boost::beast;

namespace cube::programs::http
{

void session::run()
{
    do_read(std::make_shared<life_sign>(*this));
}

session::session(tcp::socket && socket, event_poller_t & poller,
    request_handler_t request_handler,
    finished_handler_t finished_handler) :
    stream_(std::move(socket)),
    buffer_(8 * 1024),
    rw_poll_(poller, stream_.socket().native_handle()),
    request_handler_(std::move(request_handler)),
    finished_handler_(std::move(finished_handler))
{ }

void session::do_read(std::shared_ptr<life_sign> life_sign)
{
    using std::operator""s;

    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    request_ = {};

    stream_.expires_after(15s);

    // Read the request
    rw_poll_.set_events(fd_event_notifier::read);
    http::async_read(stream_, buffer_, request_,
        scoped_handler(std::bind(
            &session::on_read,
            this,
            life_sign,
            std::placeholders::_1),
        *this));
}

void session::on_read(std::shared_ptr<life_sign> life_sign, beast::error_code ec)
{
    // Client closed the connection
    if (ec == http::error::end_of_stream)
        return do_close(life_sign);
    if (ec == beast::error::timeout)
        return; // Ignore timeouts
    if (ec) {
        LOG_WRN("HTTP session failed to read", LOG_ARG("error", ec.message()));
        return;
    }

    response_ = request_handler_(std::move(request_));

    // Write the response
    rw_poll_.set_events(fd_event_notifier::write);
    http::async_write(stream_, response_,
        scoped_handler(std::bind(
            &session::on_write,
            this,
            life_sign,
            response_.need_eof(),
            std::placeholders::_1),
        *this));
}

void session::on_write(std::shared_ptr<life_sign> life_sign, bool close, beast::error_code ec)
{
    if (ec) {
        LOG_WRN("HTTP session failed to write", LOG_ARG("error", ec.message()));
        return;
    }

    if (close)
        return do_close(life_sign);

    do_read(life_sign);
}

void session::do_close(std::shared_ptr<life_sign>)
{
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // ignore the error
}


} // End of namespace
