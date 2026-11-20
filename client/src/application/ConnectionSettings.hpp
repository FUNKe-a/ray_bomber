#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <array>
#include <cstddef>
#include <utility>

enum class SettingsField : std::size_t
{
    PlayerName,
    ServerAddress,
    Port
};

class ConnectionSettings
{
private:
    using Values = std::array<std::string, 3>;

    // Values are stored in SettingsField order.
    Values values_{"Player", "127.0.0.1", "6769"};

public:
    class Memento
    {
    public:
        Memento(const Memento&) = default;

    private:
        friend class ConnectionSettings;

        explicit Memento(const Values& values)
            : savedValues_(values)
        {
        }

        Values savedValues_;
    };

    const std::string& value(SettingsField field) const
    {
        return values_.at(static_cast<std::size_t>(field));
    }

    void setValue(SettingsField field, std::string value)
    {
        values_.at(static_cast<std::size_t>(field)) = std::move(value);
    }

    [[nodiscard]] Memento save() const
    {
        return Memento{values_};
    }

    void restore(const Memento& memento)
    {
        Values restored = memento.savedValues_;
        values_.swap(restored);
    }

    bool differsFrom(const Memento& memento) const
    {
        return values_ != memento.savedValues_;
    }
};

// returns the converted port when validation succeeds, or std::nullopt with an error message.
inline std::optional<std::uint16_t> validateConnectionSettings(
    const ConnectionSettings& settings,
    std::string& error
)
{
    if (settings.value(SettingsField::PlayerName).empty() ||
        settings.value(SettingsField::PlayerName).find_first_not_of(' ') == std::string::npos)
    {
        error = "Enter a player name.";
        return std::nullopt;
    }

    if (settings.value(SettingsField::ServerAddress).empty())
    {
        error = "Enter a server address.";
        return std::nullopt;
    }

    unsigned int port = 0;

    const char* begin = settings.value(SettingsField::Port).data();
    const char* end = begin + settings.value(SettingsField::Port).size();

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