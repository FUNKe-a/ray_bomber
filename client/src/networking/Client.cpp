#include "Client.hpp"

#include <iostream>

Client::Client()
    : socket(ioContext)
{
}

bool Client::connect(
    const std::string& host,
    unsigned short port
)
{
    try
    {
        asio::ip::tcp::resolver resolver(ioContext);

        const auto endpoints = resolver.resolve(
            host,
            std::to_string(port)
        );

        asio::connect(socket, endpoints);

        connected = true;

        std::cout
            << "Connected to "
            << host
            << ":"
            << port
            << '\n';

        return true;
    }
    catch (const std::exception& exception)
    {
        connected = false;

        std::cerr
            << "Connection failed: "
            << exception.what()
            << '\n';

        return false;
    }
}

void Client::disconnect()
{
    if (!socket.is_open())
    {
        connected = false;
        return;
    }

    asio::error_code error;

    socket.shutdown(
        asio::ip::tcp::socket::shutdown_both,
        error
    );

    socket.close(error);

    connected = false;
}

bool Client::isConnected() const
{
    return connected;
}

bool Client::send(const std::string& message)
{
    if (!connected)
    {
        return false;
    }

    try
    {
        asio::write(
            socket,
            asio::buffer(message)
        );

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Send failed: "
            << exception.what()
            << '\n';

        connected = false;

        return false;
    }
}