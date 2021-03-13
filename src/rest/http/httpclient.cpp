#include <http/httpclient.h>
#include <utility>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest::Http;
using namespace boost::beast;

HttpClient::HttpClient(TcpClientManagement &management, Socket &&socket) :
    TcpClient(management, std::move(socket))
{

}

HttpClient::~HttpClient()
{
    
}

void HttpClient::stateChanged(const ClientState &state)
{
    switch(state) {
        case CS_ACTIVE:     asyncRead();    break;
        default:                            break;
    }
}

void HttpClient::asyncRead()
{
    // @Fixme: timeout yo...

    request_ = { };
    http::async_read(socket(), buffer_, request_, 
                        boost::bind(&HttpClient::httpRead, 
                        boost::static_pointer_cast<HttpClient>(shared_from_this()), 
                        boost::asio::placeholders::error));
}

void HttpClient::httpRead(error_code error)
{
    // Client closed connection
    if(http::error::end_of_stream == error) {
        terminate();
        return;
    }

    if(error) {
        terminate(false);
        return;
    }

}