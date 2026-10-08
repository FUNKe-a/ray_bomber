#pragma once

#include "GameEvent.hpp"
#include "GameTypes.hpp"
#include "envelope.pb.h"

#include <optional>
#include <string>

namespace ProtobufAdapter {
    Envelope makeJoin(const std::string& name);
    Envelope makeReady(bool ready);
    Envelope makeMove(Direction direction);
    Envelope makeGameStart();
    Envelope makeMatchReady();

    std::optional<GameEvent> decode(const Envelope& message);
}