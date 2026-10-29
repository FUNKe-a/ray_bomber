#include "UI.hpp"

#include <algorithm>
#include <string>
#include <array>
#include <cmath>

namespace
{
    constexpr std::array<int, 6> FontSizes{
        14, 16, 18, 24, 32, 40
    };

    std::array<Font, FontSizes.size()> fonts{};
    bool fontsLoaded = false;

    constexpr float TextSpacing = 0.0f;

    std::size_t fontIndex(float requestedSize)
    {
        std::size_t best = 0;

        for (std::size_t index = 1; index < FontSizes.size(); ++index)
        {
            if (std::abs(FontSizes[index] - requestedSize) <
                std::abs(FontSizes[best] - requestedSize))
            {
                best = index;
            }
        }

        return best;
    }

    float drawingSize(float requestedSize)
    {
        return static_cast<float>(
            FontSizes[fontIndex(requestedSize)]
        );
    }

    Font fontForSize(float requestedSize)
    {
        return fontsLoaded
            ? fonts[fontIndex(requestedSize)]
            : GetFontDefault();
    }

    // Remove a complete UTF-8 codepoint, rather than one byte.
    void removeLastCodepoint(std::string& text)
    {
        if (text.empty())
            return;

        std::size_t start = text.size() - 1;

        while (start > 0 &&
               (static_cast<unsigned char>(text[start]) & 0xC0) == 0x80)
        {
            --start;
        }

        text.resize(start);
    }
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
        const float size = drawingSize(
            static_cast<float>(fontSize)
        );

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

        const float size = drawingSize(
            static_cast<float>(theme.fontSize)
        );

        drawText(
            value,
            bounds.x + theme.padding,
            bounds.y + (bounds.height - size) / 2.0f,
            size,
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

        for (std::size_t index = 0; index < FontSizes.size(); ++index)
        {
            Font loaded = LoadFontEx(
                path,
                FontSizes[index],
                nullptr,
                0
            );

            if (loaded.texture.id == 0 ||
                loaded.texture.id == GetFontDefault().texture.id)
            {
                // Also releases any sizes loaded before this failure.
                unloadFont();
                return false;
            }

            fonts[index] = loaded;

            // Glyphs are drawn at their generated size.
            SetTextureFilter(
                fonts[index].texture,
                TEXTURE_FILTER_POINT
            );
        }

        fontsLoaded = true;
        return true;
    }

    void unloadFont()
    {
        for (Font& font : fonts)
        {
            if (font.texture.id != 0)
            {
                UnloadFont(font);
                font = {};
            }
        }

        fontsLoaded = false;
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
            fontForSize(fontSize),
            value.c_str(),
            Vector2{std::round(x), std::round(y)},
            drawingSize(fontSize),
            TextSpacing,
            color
        );
    }

    float measureText(std::string_view text, float fontSize)
    {
        const std::string value{text};

        return MeasureTextEx(
            fontForSize(fontSize),
            value.c_str(),
            drawingSize(fontSize),
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
        const float size = drawingSize(fontSize);
        const std::string fitted = fitText(
            text,
            bounds.width,
            size
        );

        drawText(
            fitted,
            bounds.x,
            bounds.y + (bounds.height - size) / 2.0f,
            size,
            color
        );
    }

    std::string fitText(
        std::string_view text,
        float maxWidth,
        float fontSize
    )
    {
        if (maxWidth <= 0.0f)
            return {};

        std::string value{text};

        if (measureText(value, fontSize) <= maxWidth)
            return value;

        const std::string suffix = "...";

        if (measureText(suffix, fontSize) > maxWidth)
            return {};

        while (!value.empty())
        {
            removeLastCodepoint(value);

            if (measureText(value + suffix, fontSize) <= maxWidth)
                return value + suffix;
        }

        return suffix;
    }
}