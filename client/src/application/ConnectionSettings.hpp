#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <system_error>

struct ConnectionSettings
{
    std::string serverAddress = "127.0.0.1";
    std::string port = "6769";
    std::string playerName = "Player";
};

// returns the converted port when validation succeeds, or std::nullopt with an error message.
inline std::optional<std::uint16_t> validateConnectionSettings(
    const ConnectionSettings& settings,
    std::string& error
)
{
    if (settings.playerName.empty() ||
        settings.playerName.find_first_not_of(' ') == std::string::npos)
    {
        error = "Enter a player name.";
        return std::nullopt;
    }

    if (settings.serverAddress.empty())
    {
        error = "Enter a server address.";
        return std::nullopt;
    }

    unsigned int port = 0;

    const char* begin = settings.port.data();
    const char* end = begin + settings.port.size();

    const auto result = std::from_chars(begin, end, port);

    if (result.ec != std::errc{} ||
        result.ptr != end ||
        port < 1 ||
        port > 65535)
    {
        error = "Port must be between 1 and 65535.";
        return std::nullopt;
    }

    error.clear();
    return static_cast<std::uint16_t>(port);
}