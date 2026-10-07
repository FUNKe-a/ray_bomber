#include "UI.hpp"

#include <string>

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
        // raylib expects a null-terminated string.
        const std::string label{text};

        const int width = MeasureText(label.c_str(), fontSize);

        DrawText(
            label.c_str(),
            static_cast<int>(bounds.x + (bounds.width - width) / 2),
            static_cast<int>(bounds.y + (bounds.height - fontSize) / 2),
            fontSize,
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

        DrawText(
            value.c_str(),
            static_cast<int>(bounds.x + theme.padding),
            static_cast<int>(
                bounds.y + (bounds.height - theme.fontSize) / 2
            ),
            theme.fontSize,
            theme.text
        );

        EndScissorMode();
    }

    void drawColorSwatch(Rectangle bounds, Color color)
    {
        DrawRectangleRec(bounds, color);
        DrawRectangleLinesEx(bounds, 1.0f, DARKGRAY);
    }
}