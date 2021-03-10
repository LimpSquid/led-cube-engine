#pragma once

#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

namespace Rest 
{

class TcpClient;
class TcpClientManagement
{
public:
    TcpClientManagement();
    ~TcpClientManagement();

    void join(const boost::shared_ptr<TcpClient> &client);
    void leave(const boost::shared_ptr<TcpClient> &client);

private:
    boost::unordered_set<boost::shared_ptr<TcpClient>> clients_;
    mutable std::mutex lock_;
};

}