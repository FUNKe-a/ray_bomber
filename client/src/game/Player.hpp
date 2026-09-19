#pragma once

#include <cstdint>

class Player
{
public:
    Player(std::uint8_t id, int startX, int startY);

    std::uint8_t getId() const;
    int getX() const;
    int getY() const;

    void setPosition(int x, int y);

private:
    std::uint8_t id;
    int x;
    int y;
};