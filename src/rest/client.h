#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class ClientManager;

class Client : public boost::enable_shared_from_this<Client>
{
public:
    using Pointer = boost::shared_ptr<Client>;
    using Socket = boost::asio::ip::tcp::socket;

    struct Context
    {
        ClientManager &manager;
        boost::asio::io_context &io;
    };

    static Pointer create(const Context &context);

    ~Client();

    Socket &socket();
    
    bool begin();
    void end();

private:
    static const int BUFFER_SIZE_DEFAULT;

    Client(const Context &context);

    void read(const boost::system::error_code &error, size_t bytes);
    void write(const boost::system::error_code &error, size_t bytes);
   
    ClientManager &manager_;
    boost::asio::io_context &context_;
    boost::asio::ip::tcp::socket socket_;
    std::vector<char> readBuffer_;
    std::vector<char> writeBuffer_;
};

}