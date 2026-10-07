#pragma once

#include "GameTypes.hpp"
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
    PlayerColor getColor() const;
    void setColor(PlayerColor color);
    bool isReady() const;
    void setReady(bool ready);

private:
    std::uint32_t id;
    int x = 0;
    int y = 0;
    bool positioned = false;
    std::string name;
    PlayerColor color = PlayerColor::Unknown;
    bool ready = false;
};
