#pragma once
#include "nlohmann/json.hpp"
#include "Command.hpp"
#include <unordered_map>
#include <charconv>
#include <sstream>
class Util
{
public:
    static Command deserialize(nlohmann::json &jsonCommand);
    static nlohmann::json serialize(Command command);
    static Command collectAddCommand(const std::string &trigger);
    static Command collectUpdateCommand(const std::string &trigger, const Command &command);
    static bool promptBoolWithDefault(const std::string &label, bool currentVal);
    static bool collectRemoveCommand(const std::string &trigger);
    static void printCommandDetails(const Command &command);
    static void printCommands(const std::unordered_map<std::string, Command> &commands);
    static void printHelp();
    static int collectTimeoutCommand();
    static CommandType recognizeCommand(const std::string &input);
    static bool collectReloadCommand();
    static bool collectClearCommand();
    static std::string collectTrigger(const std::string &input);
    static std::string collectFilePath(const std::string &input);
    static bool endsWith(const std::string &buffer, const std::string &trigger, bool caseSensitive);
};
