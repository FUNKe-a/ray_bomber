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

    Button::Button(std::string label, Theme theme, Action onActivated)
        : label_(std::move(label)),
          theme_(theme),
          onActivated_(std::move(onActivated))
    {
    }

    void Button::draw(bool focused) const
    {
        drawButton(bounds(), label_, isEnabled(), theme_, focused);
    }

    void Button::handleInput(const ControlInput& input, bool clicked)
    {
        if (clicked || input.activate)
            onActivated_();
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

    void LineInput::draw(bool focused) const
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
            if (&member == &control && member.isEnabled())
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
            if (candidate.isEnabled())
            {
                focused_ = &candidate;
                return;
            }
        }

        focused_ = nullptr;
    }

    void Form::update()
    {
        const ControlInput input = readControlInput();

        if (!input.controlDown && IsKeyPressed(KEY_TAB))
        {
            const bool backwards =
                IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            moveFocus(backwards);
            return;
        }

        if (!focused_ || !focused_->isEnabled())
            moveFocus(false);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            const Vector2 mouse = GetMousePosition();

            // The last drawn control is on top if bounds overlap.
            for (auto it = controls_.rbegin(); it != controls_.rend(); ++it)
            {
                Control& control = it->get();

                if (!control.contains(mouse))
                    continue;

                if (control.isEnabled())
                {
                    focused_ = &control;
                    control.handleInput(input, true);
                }

                return;
            }
        }

        if (focused_)
            focused_->handleInput(input, false);
    }

    void Form::draw() const
    {
        for (const Control& control : controls_)
            control.draw(&control == focused_ && control.isEnabled());
    }
}