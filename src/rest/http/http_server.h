#pragma once

#include <net/tcp_server.h>

namespace rest::http
{

class http_server : public rest::net::tcp_server
{
public:
    /**
     * @brief Construct a new http_server object
     * @param address The address to listen on for incoming connections
     * @param port The port to listen on for incoming connections
     */
    http_server(const std::string &address, const std::string &port);

    /**
     * @brief Construct a new http_server object
     * @param endpoint The endpoint to listen on for incoming connections
     */
    http_server(const boost::asio::ip::tcp::endpoint &endpoint);

    /**
     * @brief Destroy the http_server object
     */
    virtual ~http_server() override;

private:
    virtual rest::net::tcp_client::pointer create_client(rest::net::tcp_client_management &management, socket_type &&socket) const override;
};

}