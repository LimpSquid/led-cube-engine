#include "client.h"

using namespace Rest;

Client::Pointer Client::create(boost::asio::io_context &context)
{
    return Pointer(new Client(context));
}

Client::~Client()
{

}

Client::Socket &Client::socket()
{
    return socket_;
}

Client::Client(boost::asio::io_context &context) :
    context_(context),
    socket_(context)
{

}