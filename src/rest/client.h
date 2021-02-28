#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class Client : public boost::enable_shared_from_this<Client>
{
public:
    using Pointer = boost::shared_ptr<Client>;
    using Socket = boost::asio::ip::tcp::socket;

    static Pointer create(boost::asio::io_context &context);

    ~Client();

    Socket &socket();

private:
    Client(boost::asio::io_context &context);

    boost::asio::io_context &context_;
    boost::asio::ip::tcp::socket socket_;
};

}