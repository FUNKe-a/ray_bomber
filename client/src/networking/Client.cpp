#include "Client.hpp"

#include <exception>
#include <utility>

Client::~Client()
{
    disconnect();
}

void Client::close(const std::shared_ptr<Session>& current)
{
    if (!current) return;
    current->resolver.cancel();
    asio::error_code ignored;
    current->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    current->socket.close(ignored);
    current->outgoing.clear();
}

void Client::connect(const std::string& address, std::uint16_t port)
{
    if (connectionState == ConnectionState::Connecting || isConnected())
        return;
    disconnect();
    errorMessage.clear();
    connectionState = ConnectionState::Connecting;
    auto current = std::make_shared<Session>(ioContext);
    session = current;

    current->resolver.async_resolve(address, std::to_string(port),
        [this, current](const asio::error_code& error,
                       const asio::ip::tcp::resolver::results_type& endpoints)
        {
            if (session != current) return;
            if (error) { fail(current, error.message()); return; }
            asio::async_connect(current->socket, endpoints,
                [this, current](const asio::error_code& connectError,
                                const asio::ip::tcp::endpoint&)
                {
                    if (session != current) return;
                    if (connectError) { fail(current, connectError.message()); return; }
                    connectionState = ConnectionState::Connected;
                });
        });
}

void Client::poll()
{
    ioContext.restart();
    ioContext.poll();
}

void Client::startReceiving()
{
    if (!isConnected() || !session || session->receiving) return;
    session->receiving = true;
    readHeader(session);
}

void Client::readHeader(const std::shared_ptr<Session>& current)
{
    asio::async_read(current->socket, asio::buffer(current->header),
        [this, current](const asio::error_code& error, std::size_t count)
        {
            if (session != current) return;
            if (error) { fail(current, error.message()); return; }
            if (count != Protocol::HeaderSize)
            { fail(current, "Incomplete envelope header."); return; }
            try
            {
                current->body.resize(Protocol::decodeLength(current->header));
            }
            catch (const std::exception& exception)
            { fail(current, exception.what()); return; }
            readBody(current);
        });
}

void Client::readBody(const std::shared_ptr<Session>& current)
{
    asio::async_read(current->socket, asio::buffer(current->body),
        [this, current](const asio::error_code& error, std::size_t count)
        {
            if (session != current) return;
            if (error) { fail(current, error.message()); return; }
            Envelope message;
            if (count != current->body.size() ||
                !Protocol::deserialize(current->body, message) ||
                !Protocol::isServerMessage(message))
            { fail(current, "Invalid server envelope."); return; }
            if (incomingMessages.size() >= MaxQueuedMessages)
            { fail(current, "Incoming message queue is full."); return; }
            incomingMessages.push_back(std::move(message));
            readHeader(current);
        });
}

bool Client::receive(Envelope& message)
{
    if (incomingMessages.empty()) return false;
    message = std::move(incomingMessages.front());
    incomingMessages.pop_front();
    return true;
}

void Client::send(const Envelope& message)
{
    if (!isConnected() || !session) return;
    switch (message.payload_case())
    {
        case Envelope::kJoinLobby:
        case Envelope::kUpdateReadyState:
        case Envelope::kMoveRequest:
            break;
        default:
            fail(session, "Cannot send a server payload as a request.");
            return;
    }
    auto current = session;
    if (current->outgoing.size() >= MaxQueuedMessages)
    { fail(current, "Outgoing message queue is full."); return; }
    try
    {
        current->outgoing.push_back(
            std::make_shared<std::vector<std::uint8_t>>(Protocol::serialize(message)));
    }
    catch (const std::exception& exception)
    { fail(current, exception.what()); return; }
    if (!current->writing) writeNext(current);
}

void Client::writeNext(const std::shared_ptr<Session>& current)
{
    if (session != current) return;
    if (current->outgoing.empty()) { current->writing = false; return; }
    current->writing = true;
    // Capture the packet so its bytes survive cancellation/queue clearing.
    auto packet = current->outgoing.front();
    asio::async_write(current->socket, asio::buffer(*packet),
        [this, current, packet](const asio::error_code& error, std::size_t count)
        {
            if (session != current) return;
            if (error) { fail(current, error.message()); return; }
            if (count != packet->size())
            { fail(current, "Incomplete envelope write."); return; }
            current->outgoing.pop_front();
            writeNext(current);
        });
}

void Client::fail(const std::shared_ptr<Session>& current, std::string error)
{
    if (session != current) return;
    // Copy before resetting: the argument can alias the session member.
    auto failed = current;
    session.reset();
    close(failed);
    errorMessage = std::move(error);
    connectionState = ConnectionState::Failed;
}

void Client::disconnect()
{
    auto previous = std::move(session);
    close(previous);
    incomingMessages.clear();
    connectionState = ConnectionState::Disconnected;
}

Client::ConnectionState Client::getConnectionState() const { return connectionState; }
bool Client::isConnected() const { return connectionState == ConnectionState::Connected; }
const std::string& Client::getError() const { return errorMessage; }
