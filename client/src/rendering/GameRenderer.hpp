#pragma once

class Game;
class Map;
class Player;

class GameRenderer
{
public:
    void render(const Game& game) const;

private:
    static constexpr int TileSize = 48;
    static constexpr int PlayerPadding = 8;

    void drawMap(const Map& map) const;
    void drawPlayers(const Game& game) const;
    void drawPlayer(const Player& player, bool isLocalPlayer) const;
};