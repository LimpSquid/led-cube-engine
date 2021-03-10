#include <tcpserver.h>
#include <tcpclientmanagement.h>
#include <utility>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace Rest;
using namespace boost::asio;

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
                management_.join(shared_from_this());
                setState(CS_ACTIVE);
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
            management_.leave(shared_from_this());

            setState(CS_ABOUT_TO_TERMINATE);            
            if(graceful)
                socket_.shutdown(Socket::shutdown_send);
            else
                socket_.close();
            setState(CS_TERMINATED);  
            break;
        default:                
            break;
    }
}

TcpClient::TcpClient(TcpClientManagement &management, Socket &&socket) :
    management_(management),
    socket_(std::move(socket))
{
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

void TcpClient::stateChanged(const ClientState &)
{
    // Do nothing
}

void TcpClient::setState(const ClientState &value)
{
    if(state_ != value) {
        state_ = value;
        stateChanged(state_);
    }
}