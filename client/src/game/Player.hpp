#pragma once

class Player
{
public:
    Player(int startX, int startY);

    int getX() const;
    int getY() const;

    void setPosition(int x, int y);

private:
    int x;
    int y;
};