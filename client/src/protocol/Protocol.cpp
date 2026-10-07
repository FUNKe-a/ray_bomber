#include "Protocol.hpp"

#include <stdexcept>
#include <utility>

std::vector<std::uint8_t> Protocol::serialize(const Envelope& message)
{
    if (message.payload_case() == Envelope::PAYLOAD_NOT_SET)
        throw std::invalid_argument("Envelope has no payload.");
    if (message.ByteSizeLong() > MaxBodySize)
        throw std::runtime_error("Envelope exceeds frame limit.");

    std::string body;
    if (!message.SerializeToString(&body))
        throw std::runtime_error("Could not serialize envelope.");
    return frameBody(body);
}

bool Protocol::deserialize(
    std::span<const std::uint8_t> body, Envelope& message)
{
    if (body.empty() || body.size() > MaxBodySize)
        return false;
    Envelope parsed;
    if (!parsed.ParseFromArray(body.data(), static_cast<int>(body.size())) ||
        parsed.payload_case() == Envelope::PAYLOAD_NOT_SET)
        return false;
    message = std::move(parsed);
    return true;
}
