#include "MacroManager.hpp"

MacroManager::MacroManager(const std::unordered_map<std::string, Command> &commands)
    : commands(commands), is_dirty(0), timeoutUntil(std::chrono::steady_clock::time_point::min())
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

std::optional<Command> MacroManager::findCommand(const std::string &key)
{
    auto it = commands.find(key);
    std::optional<Command> optionalCommand = std::nullopt;

    if (it != commands.end())
    {
        optionalCommand = it->second;
    }

    return optionalCommand;
}

void MacroManager::enableCommand(const std::string &key, bool enabled, const std::string &prompt)
{
    std::optional<Command> command = findCommand(key);

    if (command)
    {
        std::string out = prompt == "enable" ? "enabled" : "disabled";
        commands[key].setEnabled(enabled);
        std::cout << "Command " << key << " " << out << std::endl;
        is_dirty = true;
        return;
    }

    std::cout << "Could not " << prompt << " command " << key << std::endl;
}

void MacroManager::caseSensitive(const std::string &key)
{
    auto it = commands.find(key);
    if (it != commands.end())
    {
        bool newState = !it->second.isCaseSensitive();
        it->second.setCaseSensitive(newState);
        is_dirty = true;
        std::cout << "Command " << key << " case sensitive changed to "
                  << (newState ? "true" : "false") << std::endl;
        return;
    }

    std::cout << "Could not change case sensitive for command " << key << std::endl;
}
void MacroManager::setCommands(const std::unordered_map<std::string, Command> &commands)
{
    this->commands = commands;
    is_dirty = true;
}

void MacroManager::clear()
{
    is_dirty = true;
    commands.clear();
}

void MacroManager::setTimeout(int mintues)
{
    if (mintues <= 0)
    {
        clearTimeout();
        return;
    }

    timeoutUntil = std::chrono::steady_clock::now() + std::chrono::minutes(mintues);
}

void MacroManager::clearTimeout()
{
    timeoutUntil = std::chrono::steady_clock::time_point::min();
}

bool MacroManager::isTimedout() const
{
    return std::chrono::steady_clock::now() < timeoutUntil;
}

long long MacroManager::getRemainingTimeoutSeconds() const
{
    if (!isTimedout())
    {
        return 0;
    }

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
        timeoutUntil - std::chrono::steady_clock::now());
    return remaining.count();
}