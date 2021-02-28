#include "server.h"
#include "client.h"
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest;
using namespace boost::asio;
using namespace boost::asio::ip;

Server::Server(const std::string &address, const std::string &port) :
    acceptor_(context_),
    endpoint_(*tcp::resolver(context_).resolve(address, port))
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

    acceptNewClient();
    return true;
}

void Server::run()
{
    if(!acceptor_.is_open())
        return;
    context_.run();
}

void Server::end()
{
    if(!acceptor_.is_open())
        return;

    acceptor_.close();
}

void Server::acceptNewClient()
{
    Client::Context context = { 
        .manager = manager_, 
        .io = context_
    };

    Client::Pointer client = Client::create(context);

    acceptor_.async_accept(client->socket(), boost::bind(&Server::acceptClient, this, client, placeholders::error));
}

void Server::acceptClient(const boost::shared_ptr<Client> &client, const boost::system::error_code &error)
{
    if(!error)
        client->begin();
    acceptNewClient();
}
