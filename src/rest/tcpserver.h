#pragma once

#include <tcpclient.h>
#include <tcpclientmanagement.h>
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class TcpServer
{
public:
    /**
     * @brief The socket type for a TcpClient object 
     */
    using Socket = TcpClient::Socket;

    /**
     * @brief Destroy the TcpServer object
     */
    virtual ~TcpServer();

    /**
     * @brief Begin the TcpServer
     * Starts the server and starts listening for incoming connections
     * @return Returns true when the server was started succesfully
     * @return Returns false when the server couldn't be started or was running already
     */
    bool begin();

    /**
     * @brief Run the TcpServer
     * Start running the server
     */
    void run();

    /**
     * @brief  End the TcpServer
     * Stop listening for incoming connections, closes down open connections and stops the server
     */
    void end();

protected:
    /**
     * @brief Construct a new TcpServer object
     * @param address The address to listen on for incoming connections
     * @param port The port to listen on for incoming connections
     */
    TcpServer(const std::string &address, const std::string &port);

    /**
     * @brief Construct a new TcpServer object
     * @param endpoint The endpoint to listen on for incoming connections
     */
    TcpServer(const boost::asio::ip::tcp::endpoint &endpoint);

    /**
     * @brief Create a TcpClient object
     * A method that must be overridden to return a TcpClient object
     * @param management The management to associate with the new client
     * @param socket The socket to associate with the new client
     * @return Returns TcpClient* 
     */
    virtual TcpClient::Pointer createClient(TcpClientManagement &management, Socket &&socket) const = 0;

private:
    void acceptNewClient();
    void acceptClient(const boost::system::error_code &error);

    boost::asio::io_context context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::endpoint endpoint_;
    TcpClientManagement management_;
    Socket socket_;
};

}