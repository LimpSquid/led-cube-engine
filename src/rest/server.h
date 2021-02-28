#pragma once

#include <string>
#include <boost/asio.hpp>

namespace Rest
{

class Server
{
public:
    Server(const std::string &address, const std::string &port);
    Server(const boost::asio::ip::tcp::endpoint &endpoint);
    ~Server();

    bool begin();
    void end();

private:
    boost::asio::io_context context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

}