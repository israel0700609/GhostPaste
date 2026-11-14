#include "CommandLoop.hpp"

void CommandLoop::handleAdd(const std::string &input)
{
    std::string trigger = Util::collectTrigger(input);
    std::optional<Command> command = m_macroManager.findCommand(trigger);
    if (command)
    {
        std::cerr << "Error: Trigger " << trigger
                  << " already exists. Use update or remove first." << std::endl;
        return;
    }
    Command newCommand = Util::collectAddCommand(trigger);
    m_macroManager.addCommand(trigger, newCommand);

    std::cout << "Added command '" << trigger << "'." << std::endl;
}

void CommandLoop::handleUpdate(const std::string &input)
{
    std::string trigger = Util::collectTrigger(input);
    std::optional<Command> oldCommand = m_macroManager.findCommand(trigger);

    if (oldCommand)
    {
        Command updatedCommand = Util::collectUpdateCommand(trigger, oldCommand.value());
        m_macroManager.updateCommand(trigger, updatedCommand);
        std::cout << "Updated command '" << trigger << "'." << std::endl;
    }
    else
    {
        std::cerr << "Error: Command '" << trigger << "' not found." << std::endl;
    }
}

void CommandLoop::handleRemove(const std::string &input)
{
    std::string trigger = Util::collectTrigger(input);
    if (!m_macroManager.findCommand(trigger))
    {
        std::cerr << "Error: Command '" << trigger << "' not found." << std::endl;
        return;
    }

    bool answer = Util::collectRemoveCommand(trigger);
    if (answer)
    {
        m_macroManager.removeCommand(trigger);
        std::cout << "Deleted command '" << trigger << "'." << std::endl;
    }
}

void CommandLoop::handleInfo(const std::string &input)
{
    std::string trigger = Util::collectTrigger(input);
    std::optional<Command> command = m_macroManager.findCommand(trigger);

    if (command)
    {
        Util::printCommandDetails(command.value());
    }
    else
    {
        std::cerr << "Error: Command '" << trigger << "' not found." << std::endl;
    }
}

void CommandLoop::handleEnable(const std::string &input, bool enabled, const std::string &prompt)
{
    std::string trigger = Util::collectTrigger(input);
    m_macroManager.enableCommand(trigger, enabled, prompt);
}

void CommandLoop::handleList()
{
    const auto &commands = m_macroManager.getCommands();
    Util::printCommands(commands);
}

void CommandLoop::handleCaseSensitive(const std::string &input)
{
    std::string trigger = Util::collectTrigger(input);
    m_macroManager.caseSensitive(trigger);
}

void CommandLoop::handleReload()
{
    try
    {
        std::unordered_map<std::string, Command> commands = m_storage.loadCommands();

        if (m_macroManager.isDirty())
        {
            std::cout << "Caution: Reloading will override unsaved changes in memory." << std::endl;
            if (!Util::collectReloadCommand())
            {
                return;
            }
        }

        m_macroManager.setCommands(commands);
        m_macroManager.clearIsDirty();
        std::cout << "Reloaded commands from disk." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reloading commands: " << e.what() << std::endl;
    }
}

void CommandLoop::handleExport(const std::string &input)
{
    std::string filePath = Util::collectFilePath(input);

    if (!filePath.empty() && filePath != "unknown")
    {
        try
        {
            StorageManager sm(filePath);
            sm.dumpCommands(m_macroManager.getCommands());
            std::cout << "Exported commands to '" << filePath << "'." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Could not export: " << e.what() << std::endl;
        }
        return;
    }

    std::cerr << "Could not export: invalid file path." << std::endl;
}

void CommandLoop::handleImport(const std::string &input)
{
    std::string filePath = Util::collectFilePath(input);

    if (!filePath.empty() && filePath != "unknown")
    {
        try
        {
            StorageManager sm(filePath);
            const auto commands = sm.loadCommands();

            m_macroManager.setCommands(commands);
            if (m_storage.dumpCommands(commands))
            {
                m_macroManager.clearIsDirty();
            }
            std::cout
                << "Imported commands from '" << filePath << "' to '" << m_storage.getFilePath() << "'." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Could not import: " << e.what() << std::endl;
        }
        return;
    }

    std::cerr << "Could not import: invalid file path." << std::endl;
}

void CommandLoop::handleClear()
{
    bool answer = Util::collectClearCommand();

    if (answer)
    {
        m_macroManager.clear();
        m_storage.clear();
        m_macroManager.clearIsDirty();
        std::cout << "Commands cleared successfully." << std::endl;
    }
}

void CommandLoop::handleTimeout()
{
    int minutes = Util::collectTimeoutCommand();
    m_macroManager.setTimeout(minutes);
    std::cout << "GhostPaste paused for " << minutes << " minute(s)." << std::endl;
}

void CommandLoop::handleUnTimeout()
{
    if (!m_macroManager.isTimedout())
    {
        std::cout << "GhostPaste is not currently paused." << std::endl;
        return;
    }
    m_macroManager.clearTimeout();
    std::cout << "Timeout cancelled. GhostPaste is active again." << std::endl;
}

void CommandLoop::start()
{
    std::string input;

    while (true)
    {
        std::cout << "GhostPaste >> ";
        if (!std::getline(std::cin, input))
        {
            break;
        }

        if (input.empty())
        {
            continue;
        }

        CommandType type = Util::recognizeCommand(input);

        switch (type)
        {
        case CommandType::ADD:
            handleAdd(input);
            break;
        case CommandType::UPDATE:
            handleUpdate(input);
            break;
        case CommandType::REMOVE:
            handleRemove(input);
            break;
        case CommandType::INFO:
            handleInfo(input);
            break;
        case CommandType::LIST:
            handleList();
            break;
        case CommandType::ENABLE:
            handleEnable(input, true, "enable");
            break;
        case CommandType::DISABLE:
            handleEnable(input, false, "disable");
            break;
        case CommandType::CASE_SENSITIVE:
            handleCaseSensitive(input);
            break;
        case CommandType::TIMEOUT:
            handleTimeout();
            break;
        case CommandType::UNTIMEOUT:
            handleUnTimeout();
            break;
        case CommandType::RELOAD:
            handleReload();
            break;
        case CommandType::EXPORT:
            handleExport(input);
            break;
        case CommandType::IMPORT:
            handleImport(input);
            break;
        case CommandType::CLEAR:
            handleClear();
            break;
        case CommandType::HELP:
            Util::printHelp();
            break;
        case CommandType::EXIT:
            return;
        case CommandType::UNKNOWN:
        default:
            std::cerr << "Could not recognize command type. Type /help for options." << std::endl;
            break;
        }
    }
}
