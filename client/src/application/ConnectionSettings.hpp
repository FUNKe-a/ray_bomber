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

inline std::optional<std::uint16_t> parseConnectionPort(const std::string& text)
{
    if (text.empty())
        return std::nullopt;

    unsigned int port = 0;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto result = std::from_chars(begin, end, port);

    if (result.ec != std::errc{} || result.ptr != end ||
        port == 0 || port > 65535)
    {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(port);
}

inline const char* settingsFieldError(
    const ConnectionSettings& settings,
    SettingsField field
)
{
    const auto& value = settings.value(field);

    switch (field)
    {
        case SettingsField::PlayerName:
            if (value.find_first_not_of(' ') == std::string::npos)
                return "Enter a player name.";
            break;

        case SettingsField::ServerAddress:
            if (value.empty())
                return "Enter a server address.";
            break;

        case SettingsField::Port:
            if (!parseConnectionPort(value))
                return "Port must be between 1 and 65535.";
            break;
    }

    return nullptr;
}

// returns the converted port when validation succeeds, or std::nullopt with an error message.
inline std::optional<std::uint16_t> validateConnectionSettings(
    const ConnectionSettings& settings,
    std::string& error
)
{
    for (const SettingsField field : {
             SettingsField::PlayerName,
             SettingsField::ServerAddress,
             SettingsField::Port
         })
    {
        if (const char* message = settingsFieldError(settings, field))
        {
            error = message;
            return std::nullopt;
        }
    }

    error.clear();
    return parseConnectionPort(settings.value(SettingsField::Port));
}