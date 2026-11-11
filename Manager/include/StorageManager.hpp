#pragma once
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include "Utils.hpp"
#include "Command.hpp"

class StorageManager
{

private:
    std::filesystem::path file_path;
    std::fstream config_file;
    nlohmann::json json_config_file;

public:
    StorageManager(std::filesystem::path filePath);
    ~StorageManager();

    bool fileExists(std::filesystem::path filePath) const;
    void validateStream();

    std::unordered_map<std::string, Command> loadCommands();
    bool dumpCommands(const std::unordered_map<std::string, Command> &commands);
};