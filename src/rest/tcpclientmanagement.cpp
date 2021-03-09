#include <tcpclientmanagement.h>

using namespace Rest;

TcpClientManagement::TcpClientManagement()
{

}

TcpClientManagement::~TcpClientManagement()
{

}

void TcpClientManagement::join(const boost::shared_ptr<TcpClient> &client)
{
    std::lock_guard locker(lock_);
    clients_.insert(client);
}

void TcpClientManagement::leave(const boost::shared_ptr<TcpClient> &client)
{
    std::lock_guard locker(lock_);
    clients_.erase(client);
}