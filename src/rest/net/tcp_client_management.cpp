#include <net/tcp_client_management.h>

using namespace rest::net;

tcp_client_management::tcp_client_management()
{

}

tcp_client_management::~tcp_client_management()
{

}


boost::shared_ptr<routing::router> tcp_client_management::router() const
{
    return router_;
}

void tcp_client_management::set_router(const boost::shared_ptr<routing::router> &value)
{
    router_ = value;
}

void tcp_client_management::join(const boost::shared_ptr<tcp_client> &client)
{
    std::lock_guard locker(lock_);
    clients_.insert(client);
}

void tcp_client_management::leave(const boost::shared_ptr<tcp_client> &client)
{
    std::lock_guard locker(lock_);
    clients_.erase(client);
}