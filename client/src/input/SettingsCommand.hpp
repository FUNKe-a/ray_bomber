#pragma once

#include "ConnectionSettings.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <optional>

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
    ChangeTextCommand(
        ConnectionSettings& settings,
        SettingsField field,
        std::string newValue
    )
        : settings(settings),
          field(field),
          previousValue(settings.value(field)),
          newValue(std::move(newValue))
    {
    }

    void execute() override
    {
        settings.setValue(field, newValue);
    }

    void undo() override
    {
        settings.setValue(field, previousValue);
    }

private:
    ConnectionSettings& settings;
    SettingsField field;
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

    void clear()
    {
        entries.clear();
    }

private:
    struct Entry
    {
        SettingsField field;
        std::unique_ptr<SettingsCommand> command;
    };

    std::vector<Entry> entries;
};