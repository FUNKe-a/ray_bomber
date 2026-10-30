#include "Client.hpp"

#include <iostream>
#include <utility>
#include <memory>

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

bool Client::send(const Protocol::Message& message)
{
    if (!connected)
    {
        return false;
    }

    try
    {
        auto packet =
            std::make_shared<std::vector<std::uint8_t>>(
                Protocol::serialize(message)
            );

        asio::async_write(
            socket,
            asio::buffer(*packet),
            [packet](
                const asio::error_code& error,
                std::size_t
            )
            {
                if (error)
                {
                    std::cerr
                        << "Send failed: "
                        << error.message()
                        << '\n';
                }
            }
        );

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Send failed: "
            << exception.what()
            << '\n';

        return false;
    }
}

void Client::startReceiving()
{
    if (!connected)
    {
        return;
    }

    readHeader();
}

void Client::poll()
{
    ioContext.poll();
}

void Client::readHeader()
{
    asio::async_read(
        socket,
        asio::buffer(headerBuffer),
        [this](
            const asio::error_code& error,
            std::size_t
        )
        {
            if (error)
            {
                if (error != asio::error::operation_aborted)
                {
                    std::cerr
                        << "Receive header failed: "
                        << error.message()
                        << '\n';
                }

                connected = false;
                return;
            }

            const auto messageType =
                static_cast<Protocol::MessageType>(
                    headerBuffer[0]
                );

            const std::uint16_t bodyLength =
                (static_cast<std::uint16_t>(headerBuffer[1]) << 8) |
                static_cast<std::uint16_t>(headerBuffer[2]);

            bodyBuffer.resize(bodyLength);

            if (bodyLength == 0)
            {
                Protocol::Message message{
                    messageType,
                    {}
                };

                receivedMessages.push(std::move(message));

                readHeader();
                return;
            }

            readBody(messageType);
        }
    );
}

void Client::readBody(Protocol::MessageType messageType)
{
    asio::async_read(
        socket,
        asio::buffer(bodyBuffer),
        [this, messageType](
            const asio::error_code& error,
            std::size_t
        )
        {
            if (error)
            {
                if (error != asio::error::operation_aborted)
                {
                    std::cerr
                        << "Receive body failed: "
                        << error.message()
                        << '\n';
                }

                connected = false;
                return;
            }

            Protocol::Message message{
                messageType,
                std::move(bodyBuffer)
            };

            receivedMessages.push(std::move(message));

            bodyBuffer.clear();

            readHeader();
        }
    );
}

bool Client::receive(Protocol::Message& message)
{
    if (receivedMessages.empty())
    {
        return false;
    }

    message = std::move(receivedMessages.front());

    receivedMessages.pop();

    return true;
}