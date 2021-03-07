#include <http/httpserver.h>
#include <http/httpclient.h>

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

Rest::TcpClient::Pointer HttpServer::createClient(TcpClientManager &manager, boost::asio::io_context &context) const
{
    return Rest::TcpClient::Pointer(new HttpClient({ manager, context }));
}
