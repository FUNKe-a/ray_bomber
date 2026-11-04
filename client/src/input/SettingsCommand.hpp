#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class SettingsCommand
{
public:
    virtual ~SettingsCommand() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
};

class ChangeTextCommand final : public SettingsCommand
{
public:
    ChangeTextCommand(std::string& target, std::string newValue)
        : target(target),
          previousValue(target),
          newValue(std::move(newValue))
    {
    }

    void execute() override
    {
        target = newValue;
    }

    void undo() override
    {
        target = previousValue;
    }

private:
    std::string& target;
    std::string previousValue;
    std::string newValue;
};

class SettingsCommandHistory
{
public:
    void execute(std::unique_ptr<SettingsCommand> command)
    {
        if (!command)
            return;

        // Allocate history storage before changing the setting.
        commands.push_back(std::move(command));

        try
        {
            commands.back()->execute();
        }
        catch (...)
        {
            commands.pop_back();
            throw;
        }
    }

    bool undo()
    {
        if (commands.empty())
            return false;

        commands.back()->undo();
        commands.pop_back();
        return true;
    }

    bool canUndo() const
    {
        return !commands.empty();
    }

private:
    std::vector<std::unique_ptr<SettingsCommand>> commands;
};