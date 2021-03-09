#include <tcpserver.h>
#include <tcpclientmanagement.h>
#include <requestparser.h>
#include <utility>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest;
using namespace boost::asio;

const int TcpClient::BUFFER_SIZE_DEFAULT = 2048;

TcpClient::~TcpClient()
{
    terminate();
}

TcpClient::ClientState TcpClient::state() const
{
    return state_;
}

void TcpClient::activate()
{
    switch(state_) {
        case CS_ACTIVE_WAIT:
            if(socket_.is_open()) {
                state_ = CS_ACTIVE;
                management_.join(shared_from_this());
                asyncRead();
            }
            break;
        default:                
            break;
    }
}

void TcpClient::terminate(bool graceful)
{
    switch(state_) {
        case CS_ACTIVE:
            state_ = CS_TERMINATED;
            management_.leave(shared_from_this());
            
            if(graceful)
                socket_.shutdown(Socket::shutdown_send);
            else
                socket_.close();
            break;
        default:                
            break;
    }
}

TcpClient::TcpClient(TcpClientManagement &management, Socket &&socket) :
    management_(management),
    socket_(std::move(socket))
{
    readBuffer_.resize(BUFFER_SIZE_DEFAULT);
    state_ = CS_ACTIVE_WAIT;
}

TcpClientManagement &TcpClient::management()
{
    return management_;
}

TcpClient::Socket &TcpClient::socket()
{
    return socket_;
}

void TcpClient::asyncRead()
{
    socket_.async_read_some(boost::asio::buffer(readBuffer_), boost::bind(&TcpClient::read, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
}

void TcpClient::read(const boost::system::error_code &error, size_t bytes)
{
    if(error::eof == error) {
        terminate();
        return;
    }

    RequestParser &parser = requestParser();

    switch(parser.parse(readBuffer_.data(), bytes)) {
        default:
        case RequestParser::PS_ERROR:
            parser.reset();
            break;
        case RequestParser::PS_FINISHED:
            parser.reset();
            break;
        case RequestParser::PS_CONTINUE:
            asyncRead();
            break;
    }
}

void TcpClient::write(const boost::system::error_code &error, size_t bytes)
{
    if(error) {
        terminate();
        return;
    }
}