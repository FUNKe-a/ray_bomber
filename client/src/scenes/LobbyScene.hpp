#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;
struct ConnectionSettings;

class LobbyScene final : public Scene
{
public:
    explicit LobbyScene(
        GameSession& session,
        ConnectionSettings& settings
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    std::unique_ptr<Scene> returnToMenu(std::string error);

    GameSession& session;
    ConnectionSettings& settings;
};
