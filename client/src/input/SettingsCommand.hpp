#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <optional>

enum class SettingsField
{
    ServerAddress,
    Port,
    PlayerName
};

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
    void execute(SettingsField field, std::unique_ptr<SettingsCommand> command)
    {
        if (!command)
            return;

        entries.push_back({field, std::move(command)});

        try
        {
            entries.back().command->execute();
        }
        catch (...)
        {
            entries.pop_back();
            throw;
        }
    }

    std::optional<SettingsField> undo()
    {
        if (entries.empty())
            return std::nullopt;

        auto& entry = entries.back();
        entry.command->undo();

        const SettingsField field = entry.field;
        entries.pop_back();
        return field;
    }

    bool canUndo() const
    {
        return !entries.empty();
    }

private:
    struct Entry
    {
        SettingsField field;
        std::unique_ptr<SettingsCommand> command;
    };

    std::vector<Entry> entries;
};