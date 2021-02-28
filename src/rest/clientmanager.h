#pragma once

#include <set>
#include <mutex>
#include <boost/shared_ptr.hpp>

namespace Rest 
{

class Client;
class ClientManager
{
public:
    ClientManager();
    ~ClientManager();

    void add(const boost::shared_ptr<Client> &client);
    void remove(const boost::shared_ptr<Client> &client);

private:
    std::set<boost::shared_ptr<Client>> clients_;
    mutable std::mutex lock_;
};

}