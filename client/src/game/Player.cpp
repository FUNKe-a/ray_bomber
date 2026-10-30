#include "Player.hpp"

Player::Player(int startX, int startY)
    : x(startX),
      y(startY)
{
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