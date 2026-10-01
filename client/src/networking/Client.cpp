#include "Client.hpp"

Client::Client()
    : socket(ioContext),
      resolver(ioContext)
{
}

void Client::connect(
    const std::string& address,
    std::uint16_t port
)
{
    if (connectionState == ConnectionState::Connecting ||
        connectionState == ConnectionState::Connected)
    {
        return;
    }

    errorMessage.clear();
    connectionState = ConnectionState::Connecting;

    resolver.async_resolve(
        address,
        std::to_string(port),
        [this](
            const asio::error_code& error,
            const asio::ip::tcp::resolver::results_type& endpoints
        )
        {
            if (error)
            {
                connectionState = ConnectionState::Failed;
                errorMessage = error.message();
                return;
            }

            asio::async_connect(
                socket,
                endpoints,
                [this](
                    const asio::error_code& error,
                    const asio::ip::tcp::endpoint& endpoint
                )
                {
                    handleConnect(error, endpoint);
                }
            );
        }
    );
}

void Client::handleConnect(
    const asio::error_code& error,
    const asio::ip::tcp::endpoint&
)
{
    if (error)
    {
        connectionState = ConnectionState::Failed;
        errorMessage = error.message();

        return;
    }

    connectionState = ConnectionState::Connected;
}

void Client::poll()
{
    ioContext.poll();
    ioContext.restart();
}

void Client::startReceiving()
{
    if (!isConnected())
    {
        return;
    }

    readHeader();
}

void Client::readHeader()
{
    asio::async_read(
        socket,
        asio::buffer(readHeaderBuffer),
        [this](
            const asio::error_code& error,
            std::size_t bytesTransferred
        )
        {
            handleReadHeader(
                error,
                bytesTransferred
            );
        }
    );
}

void Client::handleReadHeader(
    const asio::error_code& error,
    std::size_t bytesTransferred
)
{
    if (error)
    {
        connectionState = ConnectionState::Failed;
        errorMessage = error.message();
        return;
    }

    if (bytesTransferred != 3)
    {
        connectionState = ConnectionState::Failed;
        errorMessage = "Invalid message header.";
        return;
    }

    const auto messageType =
        static_cast<Protocol::MessageType>(
            readHeaderBuffer[0]
        );

    const std::uint16_t bodyLength =
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(readHeaderBuffer[1]) << 8) |
            static_cast<std::uint16_t>(readHeaderBuffer[2])
        );

    readBodyBuffer.resize(bodyLength);

    if (bodyLength == 0)
    {
        incomingMessages.push_back(
            Protocol::Message{
                messageType,
                {}
            }
        );

        readHeader();
        return;
    }

    readBody();
}

void Client::readBody()
{
    asio::async_read(
        socket,
        asio::buffer(readBodyBuffer),
        [this](
            const asio::error_code& error,
            std::size_t bytesTransferred
        )
        {
            handleReadBody(
                error,
                bytesTransferred
            );
        }
    );
}

void Client::handleReadBody(
    const asio::error_code& error,
    std::size_t bytesTransferred
)
{
    if (error)
    {
        connectionState = ConnectionState::Failed;
        errorMessage = error.message();
        return;
    }

    if (bytesTransferred != readBodyBuffer.size())
    {
        connectionState = ConnectionState::Failed;
        errorMessage = "Invalid message body.";
        return;
    }

    const auto messageType =
        static_cast<Protocol::MessageType>(
            readHeaderBuffer[0]
        );

    incomingMessages.push_back(
        Protocol::Message{
            messageType,
            std::move(readBodyBuffer)
        }
    );

    readBodyBuffer.clear();

    readHeader();
}

bool Client::receive(Protocol::Message& message)
{
    if (incomingMessages.empty())
    {
        return false;
    }

    message = std::move(
        incomingMessages.front()
    );

    incomingMessages.pop_front();

    return true;
}

void Client::send(const Protocol::Message& message)
{
    if (!isConnected())
    {
        return;
    }

    auto packet =
        std::make_shared<std::vector<std::uint8_t>>(
            Protocol::serialize(message)
        );

    outgoingMessages.push_back(packet);

    if (!writing)
    {
        writeNext();
    }
}

void Client::writeNext()
{
    if (outgoingMessages.empty())
    {
        writing = false;
        return;
    }

    writing = true;

    const auto& packet =
        outgoingMessages.front();

    asio::async_write(
        socket,
        asio::buffer(*packet),
        [this](
            const asio::error_code& error,
            std::size_t bytesTransferred
        )
        {
            handleWrite(
                error,
                bytesTransferred
            );
        }
    );
}

void Client::handleWrite(
    const asio::error_code& error,
    std::size_t
)
{
    if (error)
    {
        connectionState = ConnectionState::Failed;
        errorMessage = error.message();

        outgoingMessages.clear();
        writing = false;

        return;
    }

    outgoingMessages.pop_front();

    writeNext();
}

Client::ConnectionState Client::getConnectionState() const
{
    return connectionState;
}

bool Client::isConnected() const
{
    return connectionState == ConnectionState::Connected;
}

const std::string& Client::getError() const
{
    return errorMessage;
}

void Client::disconnect()
{
    resolver.cancel();

    asio::error_code error;

    socket.shutdown(
        asio::ip::tcp::socket::shutdown_both,
        error
    );

    socket.close(error);

    incomingMessages.clear();
    outgoingMessages.clear();

    writing = false;

    connectionState =
        ConnectionState::Disconnected;
}
