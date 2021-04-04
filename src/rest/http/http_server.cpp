#include <http/http_server.h>
#include <http/http_client.h>
#include <utility>
#include <boost/bind/bind.hpp>

using namespace rest::net;
using namespace rest::http;
using namespace boost::asio::ip;
using namespace boost::placeholders;

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

void http_server::install_plugin(const request_plugin &plugin)
{
    request_plugins_.push_back(plugin);
}

void http_server::install_plugin(const response_plugin &plugin)
{
    response_plugins_.push_back(plugin);
}

tcp_client::pointer http_server::create_client(tcp_client_management &management, socket_type &&socket)
{
    http_client *client = new http_client(management, std::move(socket));

    client->signal_request.connect(boost::bind(&http_server::on_signal_request, this, _1));
    client->signal_response.connect(boost::bind(&http_server::on_signal_response, this, _1, _2));
    return tcp_client::pointer(client);
}

void http_server::on_signal_request(const request_type &request)
{
    for(const request_plugin &plugin : request_plugins_)
        plugin(request);
}

void http_server::on_signal_response(const request_type &request, const response_type &response)
{
    for(const response_plugin &plugin : response_plugins_)
        plugin(request, response);
}
