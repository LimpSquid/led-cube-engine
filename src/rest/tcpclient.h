#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class RequestParser;
class TcpClientManagement;
class TcpClient : public boost::enable_shared_from_this<TcpClient>
{
public:
    enum ClientState
    {
        CS_ACTIVE_WAIT = 0,
        CS_ACTIVE,
        CS_TERMINATED,
    };

    /**
     * @brief The pointer type for a TcpClient object
     */
    using Pointer = boost::shared_ptr<TcpClient>;

    /**
     * @brief The socket type for a TcpClient object 
     */
    using Socket = boost::asio::ip::tcp::socket;

    /**
     * @brief Destroy the TcpClient object
     */
    virtual ~TcpClient();

    /**
     * @brief Get the state of the TcpClient 
     * @return Returns ClientState 
     */
    ClientState state() const;
    
    /**
     * @brief Activate the TcpClient
     * Activates the client and allows for reading and writing data from and to the socket
     */
    void activate();

    /**
     * @brief Terminate the TcpClient
     * @param graceful Indication of a graceful shutdown
     */
    void terminate(bool graceful = true);

protected:
    /**
     * @brief Construct a new TcpClient object
     * @param management The management associated to this client
     * @param socket The socket for this client
     */
    TcpClient(TcpClientManagement &management, Socket &&socket);

    /**
     * @brief Get the TcpClient its management
     * @return Returns TcpClientManagement& 
     */
    TcpClientManagement &management();

    /**
     * @brief Get the TcpClient its socket
     * @return Returns Socket& 
     */
    Socket &socket();

    /**
     * @brief Get the request parser of this TcpClient object
     * A method that must be overridden to return a reference to a RequestParser object
     * @return Returns RequestParser& 
     */
    virtual RequestParser &requestParser() = 0;

private:
    static const int BUFFER_SIZE_DEFAULT;

    void asyncRead();

    void read(const boost::system::error_code &error, size_t bytes);
    void write(const boost::system::error_code &error, size_t bytes);

    TcpClientManagement &management_;
    Socket socket_;
    std::vector<char> readBuffer_;
    std::vector<char> writeBuffer_;
    ClientState state_;
};

}