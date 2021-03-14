#include <net/tcp_server.h>
#include <routing/router_node.h>
#include <utility>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace rest::net;
using namespace rest::routing;
using namespace boost::asio;
using namespace boost::asio::ip;

tcp_server::~tcp_server()
{
    
}

bool tcp_server::begin(const boost::shared_ptr<router> &router)
{
    if(acceptor_.is_open())
        return false;

    router_ = router;
    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();

    accept_new_client();
    return true;
}

void tcp_server::run()
{
    if(!acceptor_.is_open())
        return;
    context_.run();
}

void tcp_server::end()
{
    if(!acceptor_.is_open())
        return;

    acceptor_.close();
}

tcp_server::tcp_server(const std::string &address, const std::string &port) :
    acceptor_(context_),
    endpoint_(*tcp::resolver(context_).resolve(address, port)),
    socket_(context_)
{
    
}

tcp_server::tcp_server(const tcp::endpoint &endpoint) :
    acceptor_(context_),
    endpoint_(endpoint),
    socket_(context_)
{

}

void tcp_server::accept_new_client()
{
    acceptor_.async_accept(socket_, boost::bind(&tcp_server::accept_client, this, placeholders::error));
}

void tcp_server::accept_client(const boost::system::error_code &error)
{
    if(error) 
        return;
    
    create_client(management_, std::move(socket_))->activate();
    accept_new_client();
}
