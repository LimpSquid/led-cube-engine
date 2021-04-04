#include <net/tcp_server.h>
#include <net/tcp_client_management.h>
#include <utility>

using namespace rest::net;

tcp_client::~tcp_client()
{
    terminate();
}

tcp_client::client_state tcp_client::state() const
{
    return state_;
}

void tcp_client::activate()
{
    switch(state_) {
        case cs_active_wait:
            if(socket_.is_open()) {
                management_.join(shared_from_this());
                set_state(cs_active);
            }
            break;
        default:                
            break;
    }
}

void tcp_client::terminate(bool graceful)
{
    switch(state_) {
        case cs_active:
            management_.leave(shared_from_this());

            set_state(cs_about_to_terminate);            
            if(graceful)
                socket_.shutdown(socket_type::shutdown_send);
            else
                socket_.close();
            set_state(cs_terminated);  
            break;
        default:                
            break;
    }
}

tcp_client::tcp_client(tcp_client_management &management, socket_type &&socket) :
    management_(management),
    socket_(std::move(socket))
{
    state_ = cs_active_wait;
}

tcp_client_management &tcp_client::management()
{
    return management_;
}

tcp_client::socket_type &tcp_client::socket()
{
    return socket_;
}

void tcp_client::state_changed(const client_state &)
{
    // Do nothing
}

void tcp_client::set_state(const client_state &value)
{
    if(state_ != value) {
        state_ = value;
        state_changed(state_);
    }
}