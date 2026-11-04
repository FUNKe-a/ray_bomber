#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace Protocol
{
    inline constexpr std::size_t HeaderSize = 4;
    inline constexpr std::uint32_t MaxBodySize = 1024 * 1024;

    inline std::uint32_t decodeLength(
        const std::array<std::uint8_t, HeaderSize>& header)
    {
        const std::uint32_t length =
            (std::uint32_t{header[0]} << 24) |
            (std::uint32_t{header[1]} << 16) |
            (std::uint32_t{header[2]} << 8) |
            std::uint32_t{header[3]};
        if (length == 0 || length > MaxBodySize)
            throw std::runtime_error("Invalid envelope frame length.");
        return length;
    }

    inline std::vector<std::uint8_t> frameBody(std::string_view body)
    {
        if (body.empty() || body.size() > MaxBodySize)
            throw std::runtime_error("Envelope body exceeds permitted size.");
        const auto length = static_cast<std::uint32_t>(body.size());
        std::vector<std::uint8_t> packet{
            static_cast<std::uint8_t>(length >> 24),
            static_cast<std::uint8_t>(length >> 16),
            static_cast<std::uint8_t>(length >> 8),
            static_cast<std::uint8_t>(length)};
        packet.insert(packet.end(), body.begin(), body.end());
        return packet;
    }
}
