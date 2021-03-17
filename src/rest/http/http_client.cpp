#include <http/http_client.h>
#include <http/http_handler.h>
#include <net/tcp_client_management.h>
#include <utility>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/placeholders.hpp>

using namespace rest::http;
using namespace rest::net;
using namespace boost;

http_client::http_client(tcp_client_management &management, socket_type &&socket) :
    tcp_client(management, std::move(socket))
{
    router_ = management.router();
}

http_client::~http_client()
{
    
}

void http_client::state_changed(const client_state &state)
{
    switch(state) {
        case cs_active:     async_read();   break;
        default:                            break;
    }
}

void http_client::async_read()
{
    // @Fixme: timeout yo...

    request_ = { };
    beast::http::async_read(socket(), buffer_, request_,
                                boost::bind(&http_client::http_read,
                                boost::static_pointer_cast<http_client>(shared_from_this()),
                                boost::asio::placeholders::error));
}

void http_client::async_write(response_type &&response)
{
    auto response_ptr = boost::make_shared<response_type>(std::move(response));
    beast::http::async_write(socket(), *response_ptr,
                                boost::bind(&http_client::http_write,
                                boost::static_pointer_cast<http_client>(shared_from_this()),
                                boost::asio::placeholders::error, response_ptr));
}

void http_client::http_read(beast::error_code error)
{
    // Client closed connection
    if(beast::http::error::end_of_stream == error) {
        terminate();
        return;
    }

    if(error) {
        terminate(false);
        return;
    }

    struct
    {
        http_client &client;

        void operator()(response_type &&response) const
        {
            client.async_write(std::move(response));
        }
    }
    const send_lambda = { *this };
    const request_type request = std::move(request_);
    const string_view &url_view = request.base().target();
    const regex url_regex("\\/[a-zA-Z0-9_\\-\\/]+");
    const std::string url(url_view.data(), url_view.size());

    // Generic response generators
    const auto bad_request = [&request, &send_lambda](std::string &&why)
    {
        response_type response(beast::http::status::bad_request, request.version());
        // @Todo: server field
        response.set(beast::http::field::content_type, "text/plain");
        response.keep_alive(request.keep_alive());
        response.body() = std::move(why);
        response.prepare_payload();
        send_lambda(std::move(response));
    };

    const auto not_found = [&request, &send_lambda](std::string &&why)
    {
        response_type response(beast::http::status::not_found, request.version());
        // @Todo: server field
        response.set(beast::http::field::content_type, "text/plain");
        response.keep_alive(request.keep_alive());
        response.body() = std::move(why);
        response.prepare_payload();
        send_lambda(std::move(response));
    };

    const auto server_error = [&request, &send_lambda](const std::string &why)
    {
        response_type response(beast::http::status::bad_request, request.version());
        // @Todo: server field
        response.set(beast::http::field::content_type, "text/plain");
        response.keep_alive(request.keep_alive());
        response.body() = std::move(why);
        response.prepare_payload();
        send_lambda(std::move(response));
    };

    const auto send_response = [&request, &send_lambda](response_type &&response)
    {
        // @Todo: server field
        response.keep_alive(request.keep_alive());
        response.prepare_payload();
        send_lambda(std::move(response));
    };

    if(url.empty() || !regex_match(url, url_regex))
        bad_request("Requested url has an invalid format");
    else {
        response_type response_;

        if(router_->handle<http_handler>(std::string(url.data(), url.size()), request, response_))
            send_response(std::move(response_));
        else
            not_found("Requested url does not exist");
    }
}

void http_client::http_write(beast::error_code error, boost::shared_ptr<response_type> response)
{
    // We should close the connection as specified by the response
    if(response->need_eof()) {
        terminate();
        return;
    }

    if(error) {
        terminate(false);
        return;
    }

    async_read();
}