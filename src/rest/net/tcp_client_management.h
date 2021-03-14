#pragma once

#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

namespace rest::net
{

namespace routing
{
    class router;
}

class tcp_client;
class tcp_client_management
{
public:
    tcp_client_management();
    ~tcp_client_management();

    boost::shared_ptr<routing::router> router() const;
    void set_router(const boost::shared_ptr<routing::router> &value);

    void join(const boost::shared_ptr<tcp_client> &client);
    void leave(const boost::shared_ptr<tcp_client> &client);

private:
    boost::shared_ptr<routing::router> router_;
    boost::unordered_set<boost::shared_ptr<tcp_client>> clients_;
    mutable std::mutex lock_;
};

}