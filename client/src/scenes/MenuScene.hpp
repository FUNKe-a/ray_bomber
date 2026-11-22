#pragma once

#include "PanelScene.hpp"

#include <string>
#include <memory>

class ConnectionSettings;
class GameSession;

class MenuScene final : public PanelScene
{
public:
    MenuScene(
        GameSession& session,
        ConnectionSettings& settings,
        std::string errorMessage = ""
    );

    ~MenuScene() override;

    std::optional<SceneTransition> update() override;

private:
    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    GameSession& session;
    ConnectionSettings& settings;
    std::string errorMessage;

    struct Controls;

    void requestConnect();

    std::optional<SceneTransition> pendingTransition;
    std::unique_ptr<Controls> controls;
};
