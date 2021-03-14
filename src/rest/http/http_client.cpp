#include <http/http_client.h>
#include <net/tcp_client_management.h>
#include <net/routing/router.h>
#include <utility>
#include <boost/bind.hpp>
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
        case cs_active:     async_read();    break;
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
}