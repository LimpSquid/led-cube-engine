#include <tcpserver.h>
#include <requestparser.h>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest;
using namespace boost::asio;
using namespace boost::asio::ip;

TcpServer::~TcpServer()
{
    
}

bool TcpServer::begin()
{
    if(acceptor_.is_open())
        return false;

    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();

    acceptNewClient();
    return true;
}

void TcpServer::run()
{
    if(!acceptor_.is_open())
        return;
    context_.run();
}

void TcpServer::end()
{
    if(!acceptor_.is_open())
        return;

    acceptor_.close();
}

TcpServer::TcpServer(const std::string &address, const std::string &port) :
    acceptor_(context_),
    endpoint_(*tcp::resolver(context_).resolve(address, port))
{
    
}

TcpServer::TcpServer(const tcp::endpoint &endpoint) :
    acceptor_(context_),
    endpoint_(endpoint)
{

}

void TcpServer::acceptNewClient()
{
    TcpClient::Pointer client = createClient(manager_, context_);

    acceptor_.async_accept(client->socket(), boost::bind(&TcpServer::acceptClient, this, client, placeholders::error));
}

void TcpServer::acceptClient(const boost::shared_ptr<TcpClient> &client, const boost::system::error_code &error)
{
    if(!error)
        client->begin();
    acceptNewClient();
}
