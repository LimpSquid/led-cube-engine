#include <http/httpclient.h>
#include <utility>

using namespace Rest::Http;

HttpClient::HttpClient(TcpClientManagement &management, Socket &&socket) :
    TcpClient(management, std::move(socket))
{

}

HttpClient::~HttpClient()
{
    
}

Rest::RequestParser &HttpClient::requestParser()
{
    return parser_;
}