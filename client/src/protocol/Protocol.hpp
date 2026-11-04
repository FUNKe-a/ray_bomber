#pragma once

#include "envelope.pb.h"
#include "Frame.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace Protocol
{
    std::vector<std::uint8_t> serialize(const Envelope& message);
    bool deserialize(std::span<const std::uint8_t> body, Envelope& message);
}
