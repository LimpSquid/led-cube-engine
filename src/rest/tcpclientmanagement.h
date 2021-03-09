#pragma once

#include <set>
#include <mutex>
#include <boost/shared_ptr.hpp>

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
    std::set<boost::shared_ptr<TcpClient>> clients_;
    mutable std::mutex lock_;
};

}