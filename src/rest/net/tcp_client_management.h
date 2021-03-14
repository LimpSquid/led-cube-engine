#pragma once

#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

namespace rest::net
{

class tcp_client;
class tcp_client_management
{
public:
    tcp_client_management();
    ~tcp_client_management();

    void join(const boost::shared_ptr<tcp_client> &client);
    void leave(const boost::shared_ptr<tcp_client> &client);

private:
    boost::unordered_set<boost::shared_ptr<tcp_client>> clients_;
    mutable std::mutex lock_;
};

}