#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class RequestParser;
class TcpClientManager;
class TcpClient : public boost::enable_shared_from_this<TcpClient>
{
public:
    /**
     * @brief The pointer type for a TcpClient object
     */
    using Pointer = boost::shared_ptr<TcpClient>;

    /**
     * @brief The socket type for a TcpClient object 
     */
    using Socket = boost::asio::ip::tcp::socket;

    /**
     * @brief The TcpClient context 
     */
    struct Context
    {
        TcpClientManager &manager;
        boost::asio::io_context &io;
    };

    /**
     * @brief Destroy the TcpClient object
     */
    virtual ~TcpClient();

    /**
     * @brief Get the TcpClient its socket
     * @return Returns Socket& 
     */
    Socket &socket();
    
    /**
     * @brief Begin the TcpClient
     * Starts the client and allows for reading and writing data from and to the socket
     * @return Returns true when the client was started succesfully
     * @return Returns false when the client couldn't be started or was running already
     */
    bool begin();

    /**
     * @brief End the TcpClient
     * Stop reading and writing data from and to the socket and stops the client
     */
    void end();

protected:
    /**
     * @brief Construct a new TcpClient object
     * @param context The context for the client
     */
    TcpClient(const Context &context);

    /**
     * @brief Get the request parser of this TcpClient object
     * A method that must be overridden to return a reference to a RequestParser object
     * @return Returns RequestParser& 
     */
    virtual RequestParser &requestParser() = 0;

private:
    static const int BUFFER_SIZE_DEFAULT;

    void read(const boost::system::error_code &error, size_t bytes);
    void write(const boost::system::error_code &error, size_t bytes);

    TcpClientManager &manager_;
    boost::asio::io_context &context_;
    boost::asio::ip::tcp::socket socket_;
    std::vector<char> readBuffer_;
    std::vector<char> writeBuffer_;
};

}