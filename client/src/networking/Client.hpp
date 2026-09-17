#pragma once

#include <functional>
#include <string>
#include <queue>

#include <asio.hpp>

#include "../protocol/Protocol.hpp"

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

    bool send(const Protocol::Message& message);

    void startReceiving();

    void poll();

    bool receive(Protocol::Message& message);

private:
    void readHeader();
    void readBody(Protocol::MessageType messageType);

    std::queue<Protocol::Message> receivedMessages;

    asio::io_context ioContext;
    asio::ip::tcp::socket socket;

    std::array<std::uint8_t, 3> headerBuffer{};
    std::vector<std::uint8_t> bodyBuffer;

    bool connected = false;
};
