#include "clientmanager.h"

using namespace Rest;

ClientManager::ClientManager()
{

}

ClientManager::~ClientManager()
{

}

void ClientManager::add(const boost::shared_ptr<Client> &client)
{
    std::lock_guard(lock_);
    clients_.insert(client);
}

void ClientManager::remove(const boost::shared_ptr<Client> &client)
{
    std::lock_guard(lock_);
    clients_.erase(client);
}