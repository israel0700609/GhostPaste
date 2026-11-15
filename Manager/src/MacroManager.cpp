#include "MacroManager.hpp"

MacroManager::MacroManager(const std::unordered_map<std::string, Command> &commands)
    : commands(commands), is_dirty(0), timeoutUntil(std::chrono::steady_clock::time_point::min())
{
}

void MacroManager::addCommand(const std::string &key, const Command &command)
{
    std::lock_guard<std::mutex> lock(m_mutex);

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
    std::lock_guard<std::mutex> lock(m_mutex);

    if (commands.erase(key) > 0)
    {
        is_dirty = true;
    }
}

void MacroManager::updateCommand(const std::string &key, const Command &command)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    commands[key] = command;
    is_dirty = true;
}

const std::unordered_map<std::string, Command> MacroManager::getCommands() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return commands;
}

bool MacroManager::isDirty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return is_dirty;
}

void MacroManager::clearIsDirty()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    is_dirty = false;
}

std::optional<Command> MacroManager::findCommand(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = commands.find(key);
    if (it != commands.end())
    {
        std::string out = prompt == "enable" ? "enabled" : "disabled";
        it->second.setEnabled(enabled);
        std::cout << "Command " << key << " " << out << std::endl;
        is_dirty = true;
        return;
    }

    std::cout << "Could not " << prompt << " command " << key << std::endl;
}
void MacroManager::caseSensitive(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);

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
    std::lock_guard<std::mutex> lock(m_mutex);

    this->commands = commands;
    is_dirty = true;
}

void MacroManager::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    is_dirty = true;
    commands.clear();
}

bool MacroManager::isTimedoutUnlocked() const
{
    return std::chrono::steady_clock::now() < timeoutUntil;
}

void MacroManager::setTimeout(int mintues)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (mintues <= 0)
    {
        timeoutUntil = std::chrono::steady_clock::time_point::min();
        return;
    }

    timeoutUntil = std::chrono::steady_clock::now() + std::chrono::minutes(mintues);
}

void MacroManager::clearTimeout()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    timeoutUntil = std::chrono::steady_clock::time_point::min();
}

bool MacroManager::isTimedout() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return isTimedoutUnlocked();
}

long long MacroManager::getRemainingTimeoutSeconds() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!isTimedoutUnlocked())
    {
        return 0;
    }

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
        timeoutUntil - std::chrono::steady_clock::now());
    return remaining.count();
}

std::unordered_map<std::string, Command> MacroManager::getCommandsSnapshot() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (isTimedoutUnlocked())
    {
        return {};
    }
    return commands;
}

void MacroManager::incrementCommandUsage(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = commands.find(key);
    if (it != commands.end())
    {
        it->second.incrementUsageCount();
        is_dirty = true;
    }
}