#pragma once

#include <raylib.h>
#include <string_view>

namespace UI
{
    struct Theme
    {
        Color text = DARKGRAY;
        Color mutedText = GRAY;
        Color surface = LIGHTGRAY;

        Color button = DARKBLUE;
        Color buttonHovered = BLUE;
        Color buttonText = WHITE;

        Color border = GRAY;
        Color focus = BLUE;
        Color disabled = GRAY;

        int fontSize = 20;
        float padding = 12.0f;
        float borderWidth = 2.0f;
    };

    inline constexpr Theme DefaultTheme{};

    bool isHovered(Rectangle bounds);
    bool isClicked(Rectangle bounds, bool enabled = true);

    void drawCenteredText(
        Rectangle bounds,
        std::string_view text,
        int fontSize,
        Color color
    );

    void drawButton(
        Rectangle bounds,
        std::string_view text,
        bool enabled = true,
        const Theme& theme = DefaultTheme
    );

    void drawTextField(
        Rectangle bounds,
        std::string_view text,
        bool focused,
        const Theme& theme = DefaultTheme
    );

    void drawColorSwatch(Rectangle bounds, Color color);
}