#include <string>
#include <iostream>
#include "Command.hpp"
#include "Utils.hpp"
#include "MacroManager.hpp"
#include "StorageManager.hpp"
#include <optional>

class CommandLoop
{
private:
    MacroManager &m_macroManager;
    StorageManager &m_storage;
    bool m_isRunning = true;

    void handleAdd(const std::string &input);
    void handleUpdate(const std::string &input);
    void handleRemove(const std::string &input);
    void handleInfo(const std::string &input);
    void handleList();
    void handleEnable(const std::string &input, bool enabled, const std::string &prompt);
    void handleCaseSensitive(const std::string &input);
    void handleTimeout();
    void handleUnTimeout();
    void handleReload();
    void handleExport(const std::string &input);
    void handleImport(const std::string &input);
    void hanldeImport(const std::string &input);
    void handleClear();

public:
    CommandLoop(MacroManager &macroManager, StorageManager &storage)
        : m_macroManager(macroManager), m_storage(storage) {}

    void start();
};