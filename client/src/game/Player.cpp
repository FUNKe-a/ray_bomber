#include "Player.hpp"

Player::Player(std::uint8_t id, int startX, int startY)
    : id(id),
      x(startX),
      y(startY)
{
}

std::uint8_t Player::getId() const
{
    return id;
}

int Player::getX() const
{
    return x;
}

int Player::getY() const
{
    return y;
}

void Player::setPosition(int newX, int newY)
{
    x = newX;
    y = newY;
}