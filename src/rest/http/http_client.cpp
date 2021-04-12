#include <http/http_client.h>
#include <http/http_handler.h>
#include <net/uri.h>
#include <net/tcp_client_management.h>
#include <utility>
#include <boost/bind/bind.hpp>
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
        default:;
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

    struct send_lambda
    {
        http_client &client;
        const request_type &request;

        void operator()(response_type &&response) const
        {
            client.signal_response(request, response);
            client.async_write(std::move(response));
        }
    };

    const request_type request = std::move(request_);
    const send_lambda send_lambda = { *this, request };
    const string_view &request_uri_view = request.base().target();
    const uri request_uri({ request_uri_view.data(), request_uri_view.size() });

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

    signal_request(request);

    if(request_uri.valid() && !request_uri.path().empty()) {
        response_type response_;

        if(router_->handle<http_handler>(request_uri.path(), request, response_))
            send_response(std::move(response_));
        else
            not_found("Requested resource does not exist");
    } else
        bad_request("Requested URI is malformed");
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