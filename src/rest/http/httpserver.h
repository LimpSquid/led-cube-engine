#pragma once

#include <tcpserver.h>

namespace Rest::Http
{

class HttpServer : public TcpServer
{
public:
    /**
     * @brief Construct a new HttpServer object
     * @param address The address to listen on for incoming connections
     * @param port The port to listen on for incoming connections
     */
    HttpServer(const std::string &address, const std::string &port);

    /**
     * @brief Construct a new HttpServer object
     * @param endpoint The endpoint to listen on for incoming connections
     */
    HttpServer(const boost::asio::ip::tcp::endpoint &endpoint);

    /**
     * @brief Destroy the HttpServer object
     */
    virtual ~HttpServer() override;

private:
    virtual Rest::TcpClient::Pointer createClient(TcpClientManager &manager, boost::asio::io_context &context) const override;
};

}