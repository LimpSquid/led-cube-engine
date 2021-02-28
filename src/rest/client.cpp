#include "client.h"
#include "clientmanager.h"
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest;
using namespace boost::asio;

const int Client::BUFFER_SIZE_DEFAULT = 1024;

Client::Pointer Client::create(const Context &context)
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

bool Client::begin()
{
    manager_.add(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(readBuffer_), boost::bind(&Client::read, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
    socket_.async_write_some(boost::asio::buffer(writeBuffer_), boost::bind(&Client::write, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
    return true;
}

void Client::end()
{

}

Client::Client(const Context &context) :
    manager_(context.manager),
    context_(context.io),
    socket_(context_)
{
    readBuffer_.resize(BUFFER_SIZE_DEFAULT);
    writeBuffer_.resize(BUFFER_SIZE_DEFAULT);
}

void Client::read(const boost::system::error_code &error, size_t bytes)
{
    if(error) {
        socket_.close();
        return;
    }
}

void Client::write(const boost::system::error_code &error, size_t bytes)
{
    if(error) {
        socket_.close();
        return;
    }
}