#pragma once

#include "UI.hpp"

#include <string_view>
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

namespace UI
{
    struct ControlInput
    {
        std::string typed;
        bool backspace = false;
        bool controlDown = false;
        bool activate = false;
    };

    class Control
    {
    public:
        virtual ~Control() = default;

        void setBounds(Rectangle bounds) { bounds_ = bounds; }
        Rectangle bounds() const { return bounds_; }

        void setEnabled(bool enabled) { enabled_ = enabled; }
        bool isEnabled() const { return enabled_; }
        void setVisible(bool visible) { visible_ = visible; }
        bool isVisible() const { return visible_; }

        bool contains(Vector2 point) const;
        bool canReceiveInput() const;

        virtual void draw(bool focused, bool pressed) const = 0;
        virtual void handleInput(const ControlInput& input, bool clicked) = 0;

    private:
        Rectangle bounds_{};
        bool enabled_ = true;
        bool visible_ = true;
    };

    class Button final : public Control
    {
    public:
        using Action = std::function<void()>;

        Button(std::string label, Theme theme, Action onActivated);

        void draw(bool focused, bool pressed) const override;
        void handleInput(const ControlInput& input, bool clicked) override;
        void setLabel(std::string_view label);
        void setTheme(const Theme& theme);

    private:
        std::string label_;
        Theme theme_;
        Action onActivated_;
    };

    class LineInput final : public Control
    {
    public:
        using ReadValue = std::function<const std::string&()>;
        using RequestEdit = std::function<void(std::string)>;
        using AcceptCharacter = bool (*)(char);

        LineInput(
            std::size_t maxLength,
            AcceptCharacter accepts,
            ReadValue readValue,
            RequestEdit requestEdit
        );

        void draw(bool focused, bool pressed) const override;
        void handleInput(const ControlInput& input, bool clicked) override;

    private:
        std::size_t maxLength_;
        AcceptCharacter accepts_;
        ReadValue readValue_;
        RequestEdit requestEdit_;
    };

    class Form
    {
    public:
        using ControlRef = std::reference_wrapper<Control>;

        explicit Form(std::initializer_list<ControlRef> controls);

        void focus(Control& control);
        void update(Control* preferredFallback = nullptr);
        bool hasFocus(const Control& control) const;
        void draw() const;

    private:
        void moveFocus(bool backwards);

        std::vector<ControlRef> controls_;
        Control* focused_ = nullptr;
        Control* pressed_ = nullptr;
    };
}