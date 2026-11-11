#pragma once
#include <unordered_map>
#include "Command.hpp"
#include <iostream>

class MacroManager
{
private:
    std::unordered_map<std::string, Command> commands;
    bool is_dirty = false;

public:
    MacroManager(const std::unordered_map<std::string, Command> &commands);

    void addCommand(const std::string &key, const Command &command);
    void removeCommand(const std::string &key);
    void updateCommand(const std::string &key, const Command &updatedCommand);

    const std::unordered_map<std::string, Command> &getCommands() const;
    bool isDirty() const;
    void clearIsDirty();
};