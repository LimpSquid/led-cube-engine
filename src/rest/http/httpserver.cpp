#include <http/httpserver.h>
#include <http/httpclient.h>
#include <utility>

using namespace Rest;
using namespace Rest::Http;
using namespace boost::asio::ip;

HttpServer::HttpServer(const std::string &address, const std::string &port) :
    TcpServer(address, port)
{
    
}

HttpServer::HttpServer(const tcp::endpoint &endpoint) :
    TcpServer(endpoint)
{

}

HttpServer::~HttpServer()
{

}

TcpClient::Pointer HttpServer::createClient(TcpClientManagement &management, Socket &&socket) const
{
    return Rest::TcpClient::Pointer(new HttpClient(management, std::move(socket)));
}
