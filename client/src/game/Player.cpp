#include "Player.hpp"
#include <utility>

Player::Player(std::uint32_t id) : id(id) {}
std::uint32_t Player::getId() const { return id; }
int Player::getX() const { return x; }
int Player::getY() const { return y; }
bool Player::hasPosition() const { return positioned; }
void Player::setPosition(int newX, int newY)
{ x = newX; y = newY; positioned = true; }
const std::string& Player::getName() const { return name; }
void Player::setName(std::string value) { name = std::move(value); }
player::Color Player::getColor() const { return color; }
void Player::setColor(player::Color value) { color = value; }
bool Player::isReady() const { return ready; }
void Player::setReady(bool value) { ready = value; }
