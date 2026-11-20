#include "PanelScene.hpp"

#include <raylib.h>

namespace
{
    constexpr Color PanelColor{29, 29, 29, 255};
    constexpr Color ShadowColor{0, 0, 0, 65};

    constexpr float ShadowOffsetX = 5.0f;
    constexpr float ShadowOffsetY = 7.0f;
    constexpr float PanelRoundness = 0.04f;
    constexpr int PanelSegments = 12;
}

void PanelScene::render() const
{
    const Rectangle panel = panelBounds();

    const Rectangle shadow{
        panel.x + ShadowOffsetX,
        panel.y + ShadowOffsetY,
        panel.width,
        panel.height
    };

    DrawRectangleRounded(shadow, PanelRoundness, PanelSegments, ShadowColor);
    DrawRectangleRounded(panel, PanelRoundness, PanelSegments, PanelColor);

    drawContent(panel);
}