#pragma once

#include "PanelScene.hpp"

#include <string>

struct ConnectionSettings;
class GameSession;

class MenuScene final : public PanelScene
{
public:
    MenuScene(
        GameSession& session,
        ConnectionSettings& settings,
        std::string errorMessage = ""
    );

    std::optional<SceneTransition> update() override;

private:
    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    GameSession& session;
    ConnectionSettings& settings;
    std::string errorMessage;
};
