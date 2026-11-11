#include "MacroManager.hpp"

MacroManager::MacroManager(const std::unordered_map<std::string, Command> &commands)
    : commands(commands), is_dirty(0)
{
}

void MacroManager::addCommand(const std::string &key, const Command &command)
{
    if (commands.find(key) != commands.end())
    {
        std::cerr << "Error: Trigger " << key
                  << " already exists. Use update or remove first." << std::endl;
        return;
    }

    commands[key] = command;
    is_dirty = true;
}

void MacroManager::removeCommand(const std::string &key)
{
    if (commands.erase(key) > 0)
    {
        is_dirty = true;
    }
}

void MacroManager::updateCommand(const std::string &key, const Command &command)
{
    commands[key] = command;
    is_dirty = true;
}

const std::unordered_map<std::string, Command> &MacroManager::getCommands() const
{
    return commands;
}

bool MacroManager::isDirty() const
{
    return is_dirty;
}

void MacroManager::clearIsDirty()
{
    is_dirty = false;
}
