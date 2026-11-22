#include "Controls.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
    void erasePreviousWord(std::string& text)
    {
        while (!text.empty() && text.back() == ' ')
            text.pop_back();

        if (text.empty())
            return;

        const auto isWordCharacter = [](char ch)
        {
            return std::isalnum(static_cast<unsigned char>(ch)) != 0 ||
                   ch == '_';
        };

        const bool deletingWord = isWordCharacter(text.back());

        do
        {
            text.pop_back();
        }
        while (!text.empty() &&
               text.back() != ' ' &&
               isWordCharacter(text.back()) == deletingWord);
    }

    UI::ControlInput readControlInput()
    {
        UI::ControlInput input;
        input.controlDown =
            IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        input.backspace = IsKeyPressed(KEY_BACKSPACE);
        input.activate = !input.controlDown &&
            (IsKeyPressed(KEY_ENTER) ||
             IsKeyPressed(KEY_KP_ENTER) ||
             IsKeyPressed(KEY_SPACE));

        for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
        {
            if (!input.controlDown && ch >= 32 && ch <= 126)
                input.typed.push_back(static_cast<char>(ch));
        }

        return input;
    }
}

namespace UI
{
    bool Control::contains(Vector2 point) const
    {
        return CheckCollisionPointRec(point, bounds());
    }

    bool Control::canReceiveInput() const
    {
        return visible_ && enabled_;
    }

    Button::Button(std::string label, Theme theme, Action onActivated)
        : label_(std::move(label)),
          theme_(theme),
          onActivated_(std::move(onActivated))
    {
    }

    void Button::draw(bool focused, bool pressed) const
    {
        drawButton(bounds(), label_, isEnabled(), theme_, focused, pressed);
    }

    void Button::handleInput(const ControlInput& input, bool clicked)
    {
        if (clicked || input.activate)
            onActivated_();
    }

    void Button::setLabel(std::string_view label)
    {
        if (label_ != label)
            label_.assign(label);
    }

    void Button::setTheme(const Theme& theme)
    {
        theme_ = theme;
    }

    LineInput::LineInput(
        std::size_t maxLength,
        AcceptCharacter accepts,
        ReadValue readValue,
        RequestEdit requestEdit
    )
        : maxLength_(maxLength),
          accepts_(accepts),
          readValue_(std::move(readValue)),
          requestEdit_(std::move(requestEdit))
    {
    }

    void LineInput::draw(bool focused, bool /*pressed*/) const
    {
        drawTextField(bounds(), readValue_(), focused && isEnabled(), darkTheme());
    }

    void LineInput::handleInput(const ControlInput& input, bool /*clicked*/)
    {
        if (input.typed.empty() && !input.backspace)
            return;

        const std::string& current = readValue_();
        std::string edited = current;

        for (char ch : input.typed)
        {
            if (edited.size() < maxLength_ && accepts_(ch))
                edited.push_back(ch);
        }

        if (input.backspace && !edited.empty())
        {
            if (input.controlDown)
                erasePreviousWord(edited);
            else
                edited.pop_back();
        }

        if (edited != current)
            requestEdit_(std::move(edited));
    }

    Form::Form(std::initializer_list<ControlRef> controls)
        : controls_(controls)
    {
        moveFocus(false);
    }

    void Form::focus(Control& control)
    {
        for (Control& member : controls_)
        {
            if (&member == &control && member.canReceiveInput())
            {
                focused_ = &member;
                return;
            }
        }
    }

    void Form::moveFocus(bool backwards)
    {
        if (controls_.empty())
        {
            focused_ = nullptr;
            return;
        }

        const auto current = std::find_if(
            controls_.begin(), controls_.end(),
            [this](ControlRef control) { return &control.get() == focused_; }
        );

        const auto count = controls_.size();
        std::size_t index;

        if (current == controls_.end())
            index = backwards ? 0 : count - 1;
        else
            index = static_cast<std::size_t>(current - controls_.begin());

        for (std::size_t checked = 0; checked < count; ++checked)
        {
            if (backwards)
                index = index == 0 ? count - 1 : index - 1;
            else
                index = (index + 1) % count;

            Control& candidate = controls_[index].get();
            if (candidate.canReceiveInput())
            {
                focused_ = &candidate;
                return;
            }
        }

        focused_ = nullptr;
    }

    bool Form::hasFocus(const Control& control) const
    {
        return focused_ == &control && control.canReceiveInput();
    }

    void Form::update(Control* preferredFallback)
    {
        const ControlInput input = readControlInput();
        const Vector2 mouse = GetMousePosition();

        const bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        const bool mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
        const bool mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        // A scene shortcut may have handled the frame in which the mouse was released.
        if (pressed_ && (!pressed_->isEnabled() || (!mouseDown && !mouseReleased)))
            pressed_ = nullptr;

        if (!input.controlDown && IsKeyPressed(KEY_TAB))
        {
            pressed_ = nullptr;
            const bool backwards =
                IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            moveFocus(backwards);
            return;
        }

        const bool hadEnabledFocus = focused_ && focused_->canReceiveInput();

        if (!hadEnabledFocus)
        {
            if (preferredFallback)
                focus(*preferredFallback);  // Already checks membership and enabled state.

            if (!focused_ || !focused_->canReceiveInput())
                moveFocus(false);
        }

        if (mousePressed)
        {
            pressed_ = nullptr;

            for (auto it = controls_.rbegin(); it != controls_.rend(); ++it)
            {
                Control& control = it->get();

                if (!control.isVisible() || !control.contains(mouse))
                    continue;

                if (control.isEnabled())
                {
                    focused_ = &control;
                    pressed_ = &control;
                }

                break;
            }
        }

        if (mouseReleased)
        {
            Control* released = pressed_;
            pressed_ = nullptr;

            if (released && released == focused_ &&
                released->isEnabled() && released->contains(mouse))
            {
                released->handleInput(ControlInput{}, true);
                return;
            }
        }

        if (mousePressed || pressed_)
            return;

        if (focused_ && hadEnabledFocus)
            focused_->handleInput(input, false);
    }

    void Form::draw() const
    {
        const Vector2 mouse = GetMousePosition();

        for (const Control& control : controls_)
        {
            if(!control.isVisible())
                continue;

            const bool focused = control.isEnabled() && &control == focused_;
            const bool pressed = control.isEnabled() && &control == pressed_ &&
                                control.contains(mouse);

            control.draw(focused, pressed);
        }
    }
}