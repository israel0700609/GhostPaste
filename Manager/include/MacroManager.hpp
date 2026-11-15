#pragma once

#include <unordered_map>
#include "Command.hpp"
#include <iostream>
#include <optional>
#include <chrono>
#include <mutex>
class MacroManager
{
private:
    std::unordered_map<std::string, Command> commands;
    bool is_dirty = false;
    std::chrono::steady_clock::time_point timeoutUntil;
    mutable std::mutex m_mutex;
    bool isTimedoutUnlocked() const;

public:
    MacroManager(const std::unordered_map<std::string, Command> &commands);

    void addCommand(const std::string &key, const Command &command);
    void removeCommand(const std::string &key);
    void updateCommand(const std::string &key, const Command &updatedCommand);
    std::optional<Command> findCommand(const std::string &trigger);
    void enableCommand(const std::string &key, bool enabled, const std::string &prompt);
    void caseSensitive(const std::string &key);

    const std::unordered_map<std::string, Command> getCommands() const;
    void setCommands(const std::unordered_map<std::string, Command> &commands);
    bool isDirty() const;
    void clearIsDirty();

    void setTimeout(int minutes);
    void clearTimeout();
    bool isTimedout() const;
    long long getRemainingTimeoutSeconds() const;
    std::unordered_map<std::string, Command> getCommandsSnapshot() const;
    void incrementCommandUsage(const std::string &key);
    void clear();
};