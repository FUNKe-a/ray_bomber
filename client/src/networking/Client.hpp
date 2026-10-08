#pragma once

#include "Protocol.hpp"
#include "GameEvent.hpp"
#include "GameTypes.hpp"

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
    enum class ConnectionState { Disconnected, Connecting, Connected, Failed };

    Client() = default;
    ~Client();
    void connect(const std::string& address, std::uint16_t port);
    void poll();
    void startReceiving();
    void disconnect();
    ConnectionState getConnectionState() const;
    bool isConnected() const;
    const std::string& getError() const;
    bool receive(Envelope& message);

    void joinLobby(const std::string& name);
    void setReady(bool ready);
    void move(Direction direction);
    void requestGameStart();
    void matchReady();

    bool receive(GameEvent& event);

private:
    void send(const Envelope& message);
    // Each connection owns its async buffers. Cancelled callbacks retain the
    // old session, so reconnecting cannot reuse a buffer still owned by Asio.
    struct Session
    {
        explicit Session(asio::io_context& context)
            : socket(context), resolver(context) {}
        asio::ip::tcp::socket socket;
        asio::ip::tcp::resolver resolver;
        std::array<std::uint8_t, Protocol::HeaderSize> header{};
        std::vector<std::uint8_t> body;
        std::deque<std::shared_ptr<std::vector<std::uint8_t>>> outgoing;
        bool writing = false;
        bool receiving = false;
    };

    void readHeader(const std::shared_ptr<Session>& current);
    void readBody(const std::shared_ptr<Session>& current);
    void writeNext(const std::shared_ptr<Session>& current);
    void fail(const std::shared_ptr<Session>& current, std::string error);
    static void close(const std::shared_ptr<Session>& current);

    asio::io_context ioContext;
    std::shared_ptr<Session> session;
    std::deque<GameEvent> incomingMessages;
    ConnectionState connectionState = ConnectionState::Disconnected;
    std::string errorMessage;
    static constexpr std::size_t MaxQueuedMessages = 256;
};
