
#include "Utils.hpp"

Command Util::deserialize(nlohmann::json &jsonCommand)
{
    std::string trigger = jsonCommand.value("trigger", "undefined");
    std::string value = jsonCommand.value("value", "unknown");
    std::string description = jsonCommand.value("description", "");
    bool is_case_sensitive = jsonCommand.value("is_case_sensitive", false);
    bool enabled = jsonCommand.value("enabled", true);
    int usage_count = jsonCommand.value("usage_count", 0);

    return Command(trigger, value, description, is_case_sensitive, enabled, usage_count);
}

nlohmann::json Util::serialize(Command command)
{
    nlohmann::json json;
    json["trigger"] = command.getTrigger();
    json["value"] = command.getValue();
    json["description"] = command.getDescription();
    json["is_case_sensitive"] = command.isCaseSensitive();
    json["enabled"] = command.isEnabled();
    json["usage_count"] = command.getUsageCount();

    return json;
}

Command Util::collectAddCommand(const std::string &trigger)
{
    std::string value, description, isCaseString;
    bool is_case_sensitive = false;

    while (true)
    {
        std::cout << "value = ";
        std::getline(std::cin, value);
        if (!value.empty())
        {
            break;
        }
        std::cerr << "Error: Value cannot be empty. Please try again.\n";
    }

    std::cout << "description (optional) = ";
    std::getline(std::cin, description);

    while (true)
    {
        std::cout << "is case sensitive (true/false, default false) = ";
        std::getline(std::cin, isCaseString);

        if (isCaseString.empty() || isCaseString == "false")
        {
            is_case_sensitive = false;
            break;
        }
        if (isCaseString == "true")
        {
            is_case_sensitive = true;
            break;
        }

        std::cerr << "Error: Please enter 'true' or 'false' (or press Enter for false).\n";
    }

    return Command(trigger, value, description, is_case_sensitive, true, 0);
}

bool Util::promptBoolWithDefault(const std::string &label, bool currentVal)
{
    std::string input;
    std::string currentStr = currentVal ? "true" : "false";

    while (true)
    {
        std::cout << label << " (true/false, Enter to keep [" << currentStr << "]) = ";
        std::getline(std::cin, input);

        if (input.empty())
        {
            return currentVal;
        }
        if (input == "true")
        {
            return true;
        }
        if (input == "false")
        {
            return false;
        }

        std::cerr << "Error: Please enter 'true' or 'false' (or press Enter to keep current).\n";
    }
}

Command Util::collectUpdateCommand(const std::string &trigger, const Command &command)
{
    std::string value, description;

    std::cout << "value (Enter to keep [" << command.getValue() << "]) = ";
    std::getline(std::cin, value);
    if (value.empty())
    {
        value = command.getValue();
    }

    std::cout << "description (Enter to keep [" << command.getDescription() << "], or 'none' to clear) = ";
    std::getline(std::cin, description);

    if (description.empty())
    {
        description = command.getDescription();
    }
    else if (description == "none")
    {
        description = "";
    }
    bool is_case_sensitive = promptBoolWithDefault("is case sensitive", command.isCaseSensitive());
    bool enabled = promptBoolWithDefault("is enabled", command.isEnabled());

    return Command(trigger, value, description, is_case_sensitive, enabled, command.getUsageCount());
}

bool Util::collectRemoveCommand(const std::string &trigger)
{
    std::string answerStr;

    while (true)
    {
        std::cout << "Are you sure you want to remove command '" << trigger << "'? (y/n, Enter for n): ";
        std::getline(std::cin, answerStr);

        if (answerStr.empty() || answerStr == "n" || answerStr == "N")
        {
            return false;
        }
        if (answerStr == "y" || answerStr == "Y")
        {
            return true;
        }

        std::cerr << "Error: Please enter 'y' or 'n' (or press Enter for n).\n";
    }
}

void Util::printCommandDetails(const Command &command)
{
    std::string desc = command.getDescription().empty() ? "(none)" : command.getDescription();

    std::cout << "\n========================================\n"
              << "  Trigger        : " << command.getTrigger() << "\n"
              << "  Value          : " << command.getValue() << "\n"
              << "  Description    : " << desc << "\n"
              << "  Status         : " << (command.isEnabled() ? "Enabled" : "Disabled") << "\n"
              << "  Case Sensitive : " << (command.isCaseSensitive() ? "Yes" : "No") << "\n"
              << "  Usage Count    : " << command.getUsageCount() << "\n"
              << "========================================\n";
}

void Util::printCommands(const std::unordered_map<std::string, Command> &commands)
{
    if (commands.empty())
    {
        std::cout << "\nNo macros found. Use '/add <trigger>' to create your first one.\n";
        return;
    }

    std::cout << "\nShowing " << commands.size() << " saved macro(s):\n";
    for (const auto &[key, command] : commands)
    {
        printCommandDetails(command);
    }
}

void Util::printHelp()
{
    std::cout << "\n=========================================================================\n"
              << "                         GhostPaste - Command Guide                      \n"
              << "=========================================================================\n\n"
              << "  Macro Management:\n"
              << "    /add <trigger>              Create a new macro\n"
              << "    /update <trigger>           Edit an existing macro's fields\n"
              << "    /remove <trigger>           Delete a macro\n"
              << "    /list                       Display all saved macros\n"
              << "    /info <trigger>             Show full details & stats for a macro\n\n"
              << "  State & Behavior:\n"
              << "    /enable <trigger>           Activate a disabled macro\n"
              << "    /disable <trigger>          Temporarily turn off a macro\n"
              << "    /casensitive <trigger>      Toggle case sensitivity (true/false)\n"
              << "    /timeout <trigger> <min>    Temporarily pause a macro for X minutes\n"
              << "    /untimeout <trigger>        Cancel an active timeout immediately\n\n"
              << "  Data & Backup:\n"
              << "    /reload                     Reload macros from config.json\n"
              << "    /export <path>              Export all macros to a backup JSON file\n"
              << "    /import <path>              Import macros from an external JSON file\n"
              << "    /clear                      Delete all saved macros\n\n"
              << "  General:\n"
              << "    /help                       Show this help menu\n"
              << "    /exit                       Save and exit GhostPaste\n\n"
              << "=========================================================================\n\n";
}

int Util::collectTimeoutCommand()
{
    std::string input;
    int minutes = 0;

    while (true)
    {
        std::cout << "Enter timeout in minutes (greater than 0): ";
        std::getline(std::cin, input);

        auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), minutes);

        if (ec == std::errc() && ptr == input.data() + input.size() && minutes > 0)
        {
            return minutes;
        }

        std::cerr << "Error: Please enter a valid positive integer.\n";
    }
}

CommandType Util::recognizeCommand(const std::string &inputLine)
{
    std::stringstream ss(inputLine);
    std::string cmdWord;
    ss >> cmdWord;

    if (cmdWord.empty() || cmdWord[0] != '/')
    {
        return CommandType::UNKNOWN;
    }

    static const std::unordered_map<std::string, CommandType> commandMap = {
        {"/add", CommandType::ADD},
        {"/update", CommandType::UPDATE},
        {"/remove", CommandType::REMOVE},
        {"/list", CommandType::LIST},
        {"/info", CommandType::INFO},
        {"/enable", CommandType::ENABLE},
        {"/disable", CommandType::DISABLE},
        {"/casensitive", CommandType::CASE_SENSITIVE},
        {"/timeout", CommandType::TIMEOUT},
        {"/untimeout", CommandType::UNTIMEOUT},
        {"/reload", CommandType::RELOAD},
        {"/export", CommandType::EXPORT},
        {"/import", CommandType::IMPORT},
        {"/clear", CommandType::CLEAR},
        {"/help", CommandType::HELP},
        {"/exit", CommandType::EXIT}};

    auto it = commandMap.find(cmdWord);
    if (it != commandMap.end())
    {
        return it->second;
    }

    return CommandType::UNKNOWN;
}

bool Util::collectReloadCommand()
{
    std::string answerStr;

    while (true)
    {
        std::cout << "Are you sure you want to reload? (y/n, Enter for n): ";
        std::getline(std::cin, answerStr);

        if (answerStr.empty() || answerStr == "n" || answerStr == "N")
        {
            return false;
        }
        if (answerStr == "y" || answerStr == "Y")
        {
            return true;
        }

        std::cerr << "Error: Please enter 'y' or 'n' (or press Enter for n).\n";
    }
}

bool Util::collectClearCommand()
{
    std::string answerStr;

    while (true)
    {
        std::cout << "Are you sure you want to clear commands? (y/n, Enter for n): ";
        std::getline(std::cin, answerStr);

        if (answerStr.empty() || answerStr == "n" || answerStr == "N")
        {
            return false;
        }
        if (answerStr == "y" || answerStr == "Y")
        {
            return true;
        }

        std::cerr << "Error: Please enter 'y' or 'n' (or press Enter for n).\n";
    }
}

std::string Util::collectTrigger(const std::string &input)
{
    std::stringstream ss(input);
    std::string command, trigger;

    ss >> command;
    ss >> trigger;

    while (trigger.empty())
    {
        std::cout << "Enter trigger (e.g., !email): ";
        std::string line;
        std::getline(std::cin, line);

        std::stringstream lineStream(line);
        lineStream >> trigger;

        if (trigger.empty())
        {
            std::cerr << "Error: Trigger cannot be empty.\n";
        }
    }

    return trigger;
}

std::string Util::collectFilePath(const std::string &input)
{
    std::stringstream ss(input);
    std::string command, filePath;

    ss >> command;
    std::getline(ss, filePath);

    auto cleanPath = [](std::string &str)
    {
        size_t first = str.find_first_not_of(" \t\r\n\"");
        if (first == std::string::npos)
        {
            str.clear();
            return;
        }
        size_t last = str.find_last_not_of(" \t\r\n\"");
        str = str.substr(first, (last - first + 1));
    };

    cleanPath(filePath);

    while (filePath.empty())
    {
        std::cout << "Enter file path (e.g., backup.json): ";
        std::getline(std::cin, filePath);
        cleanPath(filePath);

        if (filePath.empty())
        {
            std::cerr << "Error: File path cannot be empty.\n";
        }
    }

    return filePath;
}