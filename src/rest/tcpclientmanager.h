#pragma once

#include <set>
#include <mutex>
#include <boost/shared_ptr.hpp>

namespace Rest 
{

class TcpClient;
class TcpClientManager
{
public:
    TcpClientManager();
    ~TcpClientManager();

    void add(const boost::shared_ptr<TcpClient> &client);
    void remove(const boost::shared_ptr<TcpClient> &client);

private:
    std::set<boost::shared_ptr<TcpClient>> clients_;
    mutable std::mutex lock_;
};

}