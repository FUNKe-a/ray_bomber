#include "UI.hpp"

#include <algorithm>
#include <string>

namespace
{
    Font interfaceFont{};
    bool ownsFont = false;

    Font currentFont()
    {
        return ownsFont ? interfaceFont : GetFontDefault();
    }

    constexpr float TextSpacing = 1.0f;
}


namespace UI
{
    bool isHovered(Rectangle bounds)
    {
        return CheckCollisionPointRec(GetMousePosition(), bounds);
    }

    bool isClicked(Rectangle bounds, bool enabled)
    {
        return enabled &&
               isHovered(bounds) &&
               IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void drawCenteredText(
        Rectangle bounds,
        std::string_view text,
        int fontSize,
        Color color
    )
    {
        const float size = static_cast<float>(fontSize);
        const float width = measureText(text, size);

        drawText(
            text,
            bounds.x + (bounds.width - width) / 2.0f,
            bounds.y + (bounds.height - size) / 2.0f,
            size,
            color
        );
    }

    void drawButton(
        Rectangle bounds,
        std::string_view text,
        bool enabled,
        const Theme& theme
    )
    {
        Color background = theme.disabled;

        if (enabled)
        {
            background = isHovered(bounds)
                ? theme.buttonHovered
                : theme.button;
        }

        DrawRectangleRec(bounds, background);

        drawCenteredText(
            bounds,
            text,
            theme.fontSize,
            theme.buttonText
        );
    }

    void drawTextField(
        Rectangle bounds,
        std::string_view text,
        bool focused,
        const Theme& theme
    )
    {
        const std::string value{text};

        DrawRectangleRec(bounds, theme.surface);

        DrawRectangleLinesEx(
            bounds,
            theme.borderWidth,
            focused ? theme.focus : theme.border
        );

        // Keep long values inside the field.
        BeginScissorMode(
            static_cast<int>(bounds.x + theme.padding),
            static_cast<int>(bounds.y + theme.borderWidth),
            static_cast<int>(bounds.width - 2 * theme.padding),
            static_cast<int>(bounds.height - 2 * theme.borderWidth)
        );

        drawText(
            value,
            bounds.x + theme.padding,
            bounds.y + (bounds.height - theme.fontSize) / 2.0f,
            static_cast<float>(theme.fontSize),
            theme.text
        );

        EndScissorMode();
    }

    void drawColorSwatch(Rectangle bounds, Color color)
    {
        DrawRectangleRec(bounds, color);
        DrawRectangleLinesEx(bounds, 1.0f, DARKGRAY);
    }

    bool loadFont(const char* path)
    {
        unloadFont();

        if (!FileExists(path))
            return false;

        Font loaded = LoadFontEx(path, 48, nullptr, 0);

        // raylib can return its default font when loading fails.
        if (loaded.texture.id == 0 ||
            loaded.texture.id == GetFontDefault().texture.id)
        {
            return false;
        }

        interfaceFont = loaded;
        ownsFont = true;

        SetTextureFilter(
            interfaceFont.texture,
            TEXTURE_FILTER_BILINEAR
        );

        return true;
    }

    void unloadFont()
    {
        if (ownsFont)
        {
            UnloadFont(interfaceFont);
        }

        interfaceFont = {};
        ownsFont = false;
    }

    void drawText(
        std::string_view text,
        float x,
        float y,
        float fontSize,
        Color color
    )
    {
        const std::string value{text};

        DrawTextEx(
            currentFont(),
            value.c_str(),
            Vector2{x, y},
            fontSize,
            TextSpacing,
            color
        );
    }

    float measureText(std::string_view text, float fontSize)
    {
        const std::string value{text};

        return MeasureTextEx(
            currentFont(),
            value.c_str(),
            fontSize,
            TextSpacing
        ).x;
    }

    void drawFittedText(
        Rectangle bounds,
        std::string_view text,
        float fontSize,
        Color color
    )
    {
        const float width = measureText(text, fontSize);

        if (width > bounds.width && width > 0.0f)
        {
            fontSize *= bounds.width / width;
        }

        drawText(
            text,
            bounds.x,
            bounds.y + (bounds.height - fontSize) / 2.0f,
            fontSize,
            color
        );
    }
}