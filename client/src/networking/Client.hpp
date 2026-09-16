#pragma once

#include <string>

#include <asio.hpp>

class Client
{
public:
    Client();

    bool connect(
        const std::string& host,
        unsigned short port
    );

    void disconnect();

    bool isConnected() const;

    bool send(const std::string& message);

private:
    asio::io_context ioContext;
    asio::ip::tcp::socket socket;

    bool connected = false;
};