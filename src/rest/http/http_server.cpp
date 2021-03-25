#include <http/http_server.h>
#include <http/http_client.h>
#include <utility>

using namespace rest::net;
using namespace rest::http;
using namespace boost::asio::ip;

http_server::http_server(const std::string &address, const std::string &port) :
    tcp_server(address, port)
{
    
}

http_server::http_server(const tcp::endpoint &endpoint) :
    tcp_server(endpoint)
{

}

http_server::~http_server()
{

}

tcp_client::pointer http_server::create_client(tcp_client_management &management, socket_type &&socket) const
{
    http_client *client = new http_client(management, std::move(socket));

    // @Todo: connect signals

    return tcp_client::pointer(client);
}
