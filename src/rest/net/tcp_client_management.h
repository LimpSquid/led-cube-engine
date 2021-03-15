#pragma once

#include <net/routing/router.h>
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

    routing::router::pointer router() const;
    void set_router(const routing::router::pointer &value);

    void join(const boost::shared_ptr<tcp_client> &client);
    void leave(const boost::shared_ptr<tcp_client> &client);

private:
    routing::router::pointer router_;
    boost::unordered_set<boost::shared_ptr<tcp_client>> clients_;
    mutable std::mutex lock_;
};

}