#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;
class ConnectionSettings;

class MenuScene final : public Scene
{
public:
    MenuScene(
        GameSession& session,
        ConnectionSettings& settings,
        std::string errorMessage = ""
    );

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    GameSession& session;
    ConnectionSettings& settings;
    std::string errorMessage;
};
