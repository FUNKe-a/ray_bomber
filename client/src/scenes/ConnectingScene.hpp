#pragma once

#include "PanelScene.hpp"

#include <memory>

class GameSession;

class ConnectingScene final : public PanelScene
{
public:
    explicit ConnectingScene(GameSession& session);
    ~ConnectingScene() override;

    std::optional<SceneTransition> update() override;

private:
    struct Controls;

    GameSession& session;
    std::unique_ptr<Controls> controls;

    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;
};