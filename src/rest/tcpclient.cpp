#include <tcpserver.h>
#include <tcpclientmanager.h>
#include <requestparser.h>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <iostream> // @Commit: remove

using namespace Rest;
using namespace boost::asio;

const int TcpClient::BUFFER_SIZE_DEFAULT = 2048;

TcpClient::~TcpClient()
{
    end();
}

TcpClient::Socket &TcpClient::socket()
{
    return socket_;
}

bool TcpClient::begin()
{
    manager_.add(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(readBuffer_), boost::bind(&TcpClient::read, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
    socket_.async_write_some(boost::asio::buffer(writeBuffer_), boost::bind(&TcpClient::write, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
    return true;
}

void TcpClient::end()
{
    if(!socket_.is_open())
        return;

    manager_.remove(shared_from_this());
    socket_.close();
}

TcpClient::TcpClient(const Context &context) :
    manager_(context.manager),
    context_(context.io),
    socket_(context_)
{
    readBuffer_.resize(BUFFER_SIZE_DEFAULT);
}

void TcpClient::read(const boost::system::error_code &error, size_t bytes)
{
    if(error) {
        end();
        return;
    }

    // @Commit: remove
    std::cout << std::string(readBuffer_.data(), bytes);
}

void TcpClient::write(const boost::system::error_code &error, size_t bytes)
{
    if(error) {
        end();
        return;
    }
}