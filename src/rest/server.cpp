#include "server.h"

using namespace Rest;
using namespace boost::asio;
using namespace boost::asio::ip;

Server::Server(const std::string &address, const std::string &port) :
    Server(*tcp::resolver(context_).resolve(address, port))
{

}

Server::Server(const tcp::endpoint &endpoint) :
    acceptor_(context_),
    endpoint_(endpoint)
{

}

Server::~Server()
{
    
}

bool Server::begin()
{
    if(acceptor_.is_open())
        return false;

    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();
    
    return true;
}

void Server::end()
{

}
