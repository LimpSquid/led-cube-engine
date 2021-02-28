#pragma once

#include "clientmanager.h"
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>

namespace Rest
{

class Server
{
public:
    Server(const std::string &address, const std::string &port);
    Server(const boost::asio::ip::tcp::endpoint &endpoint);
    ~Server();

    bool begin();
    void run();
    void end();

private:
    void acceptNewClient();
    void acceptClient(const boost::shared_ptr<Client> &client, const boost::system::error_code &error);

    ClientManager manager_;
    boost::asio::io_context context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

}