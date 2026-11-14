#include "StorageManager.hpp"
#include <fstream>
#include <iostream>

StorageManager::StorageManager(std::filesystem::path filePath)
{
#ifdef PROJECT_SOURCE_DIR
    if (filePath.is_relative())
    {
        file_path = std::filesystem::path(PROJECT_SOURCE_DIR) / filePath;
    }
    else
    {
        file_path = filePath;
    }
#else
    file_path = filePath;
#endif

    if (!fileExists(file_path))
    {
        if (file_path.has_parent_path() && !std::filesystem::exists(file_path.parent_path()))
        {
            std::filesystem::create_directories(file_path.parent_path());
        }

        std::ofstream initFile(file_path);
        initFile << "{}";
        initFile.close();
    }

    config_file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!config_file.is_open())
    {
        throw std::runtime_error("Could not open config File(" + file_path.string() + "). Check for permissions and locks.");
    }

    readConfigToJson();
}

bool StorageManager::fileExists(std::filesystem::path filePath) const
{
    return std::filesystem::exists(filePath);
}

void StorageManager::validateStream()
{
    if (!config_file.is_open())
    {
        throw std::runtime_error("Fatal Error: Config file stream is not open.");
    }

    if (config_file.bad())
    {
        throw std::runtime_error("Fatal Error: Unrecoverable disk/stream I/O error (badbit).");
    }

    if (config_file.fail())
    {
        std::cerr << "Warning: Stream failed. Attempting to recover...\n";
        config_file.clear();

        if (config_file.fail())
        {
            throw std::runtime_error("Fatal Error: Could not recover the file stream.");
        }
    }
}

std::unordered_map<std::string, Command> StorageManager::loadCommands()
{
    validateStream();
    readConfigToJson();

    std::unordered_map<std::string, Command> commands;
    try
    {
        if (!json_config_file.is_object())
        {
            std::cerr << "Warning: config.json root is not a valid JSON object. Resetting data.\n";
            json_config_file = nlohmann::json::object();
            return commands;
        }

        for (auto &[key, jsonCommand] : json_config_file.items())
        {
            if (jsonCommand.is_object())
            {
                Command cmd = Util::deserialize(jsonCommand);
                commands[key] = cmd;
            }
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "Parse Error: Failed to parse config.json. Details: " << e.what() << "\n";
        std::cerr << "Starting with an empty command set to prevent crashing.\n";

        json_config_file = nlohmann::json::object();
    }
    return commands;
}

bool StorageManager::dumpCommands(const std::unordered_map<std::string, Command> &commands)
{
    validateStream();

    try
    {
        json_config_file = nlohmann::json::object();

        for (const auto &[key, command] : commands)
        {
            json_config_file[key] = Util::serialize(command);
        }

        config_file.close();

        std::ofstream outFile(file_path, std::ios::trunc);
        if (!outFile.is_open())
        {
            throw std::runtime_error("Failed to open file for dumping.");
        }

        outFile << json_config_file.dump(4);
        outFile.close();

        config_file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: Failed to dump commands to disk. Details: " << e.what() << "\n";
        return false;
    }
}

StorageManager::~StorageManager()
{
    try
    {
        if (config_file.is_open())
        {
            config_file.flush();
            config_file.close();
        }
    }
    catch (...)
    {
    }
}
void StorageManager::clear()
{
    dumpCommands({});
}

void StorageManager::readConfigToJson()
{
    validateStream();
    try
    {
        config_file.seekg(0, std::ios::beg);

        if (config_file.peek() == std::ifstream::traits_type::eof())
        {
            json_config_file = nlohmann::json::object();
        }
        else
        {
            config_file >> json_config_file;
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        json_config_file = nlohmann::json::object();
    }
}