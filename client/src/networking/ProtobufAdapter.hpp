#pragma once

#include "GameEvent.hpp"
#include "GameTypes.hpp"
#include "envelope.pb.h"

#include <optional>
#include <string>

namespace ProtobufAdapter {
    Envelope makeJoin(const std::string& name);
    Envelope makeReady(bool ready);
    Envelope makeMove(Direction direction, std::uint64_t roundId, std::uint64_t requestId);
    Envelope makeGameStart();
    Envelope makeMatchReady(std::uint64_t roundId);

    std::optional<GameEvent> decode(const Envelope& message);
}