#pragma once

#include <raylib.h>
#include <string_view>
#include <string>

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

    bool loadFont(const char* path);
    void unloadFont();

    void drawText(
        std::string_view text,
        float x,
        float y,
        float fontSize,
        Color color
    );

    float measureText(std::string_view text, float fontSize);

    void drawFittedText(
        Rectangle bounds,
        std::string_view text,
        float fontSize,
        Color color
    );

    std::string fitText(
        std::string_view text,
        float maxWidth,
        float fontSize
    );

    inline Theme darkTheme()
    {
        Theme theme;

        theme.text = Color{240, 240, 240, 255};
        theme.mutedText = Color{165, 165, 165, 255};
        theme.surface = Color{42, 42, 42, 255};

        theme.button = Color{62, 62, 62, 255};
        theme.buttonHovered = Color{82, 82, 82, 255};
        theme.buttonText = theme.text;

        theme.border = Color{70, 70, 70, 255};
        theme.focus = Color{230, 176, 86, 255};
        theme.disabled = Color{35, 35, 35, 255};

        theme.fontSize = 17;
        return theme;
    }

    inline Theme greenButtonTheme()
    {
        Theme theme = darkTheme();

        theme.button = Color{42, 115, 77, 255};
        theme.buttonHovered = Color{52, 140, 94, 255};

        return theme;
    }

    inline Theme amberButtonTheme()
    {
        Theme theme = darkTheme();

        theme.button = Color{180, 127, 48, 255};
        theme.buttonHovered = Color{207, 149, 61, 255};
        theme.buttonText = Color{255, 248, 233, 255};

        return theme;
    }
}