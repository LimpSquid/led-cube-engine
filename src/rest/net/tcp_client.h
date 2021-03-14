#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace rest::net
{

class tcp_client_management;
class tcp_client : public boost::enable_shared_from_this<tcp_client>
{
public:
    enum client_state
    {
        cs_active_wait = 0,
        cs_active,
        cs_about_to_terminate,
        cs_terminated,
    };

    /**
     * @brief The pointer type for a tcp_client object
     */
    using pointer = boost::shared_ptr<tcp_client>;

    /**
     * @brief The socket type for a tcp_client object 
     */
    using socket_type = boost::asio::ip::tcp::socket;

    /**
     * @brief Destroy the tcp_client object
     */
    virtual ~tcp_client();

    /**
     * @brief Get the state of the tcp_client 
     * @return Returns client_state 
     */
    client_state state() const;
    
    /**
     * @brief Activate the tcp_client
     * Activates the client and allows for reading and writing data from and to the socket
     */
    void activate();

    /**
     * @brief Terminate the tcp_client
     * @param graceful Indication of a graceful shutdown
     */
    void terminate(bool graceful = true);

protected:
    /**
     * @brief Construct a new tcp_client object
     * @param management The management associated to this client
     * @param socket The socket for this client
     */
    tcp_client(tcp_client_management &management, socket_type &&socket);

    /**
     * @brief Get the tcp_client its management
     * @return Returns tcp_client_management& 
     */
    tcp_client_management &management();

    /**
     * @brief Get the tcp_client its socket
     * @return Returns socket_type& 
     */
    socket_type &socket();

    /**
     * @brief The state changed callback
     * A method which may be overridden to do additional processing on state changes
     * @param state The new client state
     */
    virtual void state_changed(const client_state &state);

private:
    void set_state(const client_state &value);

    tcp_client_management &management_;
    socket_type socket_;
    client_state state_;
};

}