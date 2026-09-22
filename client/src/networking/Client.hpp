#pragma once

#include "Protocol.hpp"

#include <asio.hpp>

#include <array>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

class Client
{
public:
    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected,
        Failed
    };

    Client();

    void connect(
        const std::string& address,
        std::uint16_t port
    );

    void poll();

    void startReceiving();

    void disconnect();

    ConnectionState getConnectionState() const;

    bool isConnected() const;

    const std::string& getError() const;

    bool receive(Protocol::Message& message);

    void send(const Protocol::Message& message);

private:
    void handleConnect(
        const asio::error_code& error,
        const asio::ip::tcp::endpoint& endpoint
    );

    void readHeader();

    void handleReadHeader(
        const asio::error_code& error,
        std::size_t bytesTransferred
    );

    void readBody();

    void handleReadBody(
        const asio::error_code& error,
        std::size_t bytesTransferred
    );

    void writeNext();

    void handleWrite(
        const asio::error_code& error,
        std::size_t bytesTransferred
    );

    asio::io_context ioContext;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver resolver;
    
    ConnectionState connectionState = ConnectionState::Disconnected;
    std::string errorMessage;
    std::deque<Protocol::Message> incomingMessages;
    std::array<std::uint8_t, 3> readHeaderBuffer{};
    std::vector<std::uint8_t> readBodyBuffer;
    std::deque<std::shared_ptr<std::vector<std::uint8_t>>>
        outgoingMessages;

    bool writing = false;
};