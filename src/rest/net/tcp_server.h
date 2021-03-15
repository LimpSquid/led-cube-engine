#pragma once

#include <net/tcp_client.h>
#include <net/tcp_client_management.h>
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>

namespace rest::net
{
class tcp_server
{
public:
    /**
     * @brief The socket type for a tcp_client object 
     */
    using socket_type = tcp_client::socket_type;

    /**
     * @brief Destroy the tcp_server object
     */
    virtual ~tcp_server();

    /**
     * @brief Begin the tcp_server
     * Starts the server and starts listening for incoming connections
     * @param router The router to use for request handling
     * @return Returns true when the server was started succesfully
     * @return Returns false when the server couldn't be started or was running already
     */
    bool begin(const routing::router::pointer &router);

    /**
     * @brief Run the tcp_server
     * Start running the server
     */
    void run();

    /**
     * @brief  End the tcp_server
     * Stop listening for incoming connections, closes down open connections and stops the server
     */
    void end();

protected:
    /**
     * @brief Construct a new tcp_server object
     * @param address The address to listen on for incoming connections
     * @param port The port to listen on for incoming connections
     */
    tcp_server(const std::string &address, const std::string &port);

    /**
     * @brief Construct a new tcp_server object
     * @param endpoint The endpoint to listen on for incoming connections
     */
    tcp_server(const boost::asio::ip::tcp::endpoint &endpoint);

    /**
     * @brief Create a tcp_client object
     * A method that must be overridden to return a tcp_client object
     * @param management The management to associate with the new client
     * @param socket The socket to associate with the new client
     * @return Returns tcp_client* 
     */
    virtual tcp_client::pointer create_client(tcp_client_management &management, socket_type &&socket) const = 0;

private:
    void accept_new_client();
    void accept_client(const boost::system::error_code &error);

    boost::asio::io_context context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::endpoint endpoint_;
    tcp_client_management management_;
    socket_type socket_;
};

}