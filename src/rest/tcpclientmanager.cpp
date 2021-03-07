#include <tcpclientmanager.h>

using namespace Rest;

TcpClientManager::TcpClientManager()
{

}

TcpClientManager::~TcpClientManager()
{

}

void TcpClientManager::add(const boost::shared_ptr<TcpClient> &client)
{
    std::lock_guard locker(lock_);
    clients_.insert(client);
}

void TcpClientManager::remove(const boost::shared_ptr<TcpClient> &client)
{
    std::lock_guard locker(lock_);
    clients_.erase(client);
}