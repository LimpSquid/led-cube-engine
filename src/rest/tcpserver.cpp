#include <tcpserver.h>
#include <utility>
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
    endpoint_(*tcp::resolver(context_).resolve(address, port)),
    socket_(context_)
{
    
}

TcpServer::TcpServer(const tcp::endpoint &endpoint) :
    acceptor_(context_),
    endpoint_(endpoint),
    socket_(context_)
{

}

void TcpServer::acceptNewClient()
{
    acceptor_.async_accept(socket_, boost::bind(&TcpServer::acceptClient, this, placeholders::error));
}

void TcpServer::acceptClient(const boost::system::error_code &error)
{
    if(error) 
        return;
    
    createClient(management_, std::move(socket_))->activate();
    acceptNewClient();
}
