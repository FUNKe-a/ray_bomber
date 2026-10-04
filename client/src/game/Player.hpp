#pragma once

#include "player.pb.h"
#include <cstdint>
#include <string>

class Player
{
public:
    explicit Player(std::uint32_t id);
    std::uint32_t getId() const;
    int getX() const;
    int getY() const;
    bool hasPosition() const;
    void setPosition(int x, int y);
    const std::string& getName() const;
    void setName(std::string name);
    player::Color getColor() const;
    void setColor(player::Color color);
    bool isReady() const;
    void setReady(bool ready);

private:
    std::uint32_t id;
    int x = 0;
    int y = 0;
    bool positioned = false;
    std::string name;
    player::Color color = player::UNKNOWN;
    bool ready = false;
};
