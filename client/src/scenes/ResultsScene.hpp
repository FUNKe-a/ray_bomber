#pragma once

#include "PanelScene.hpp"

#include <memory>

class GameSession;

class ResultsScene final : public PanelScene
{
public:
    explicit ResultsScene(GameSession& session);
    ~ResultsScene() override;

    std::optional<SceneTransition> update() override;

private:
    struct Controls;

    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    GameSession& session;
    std::unique_ptr<Controls> controls;
};