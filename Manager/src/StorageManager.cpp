#include "StorageManager.hpp"

StorageManager::StorageManager(std::filesystem::path filePath)
{
    if (!fileExists(filePath))
    {
        if (filePath.has_parent_path() && !std::filesystem::exists(filePath.parent_path()))
        {
            std::filesystem::create_directories(filePath.parent_path());
        }

        std::ofstream initFile(filePath);
        initFile << "{}";
        initFile.close();
    }

    config_file.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!config_file.is_open())
    {
        throw std::runtime_error("Could not open config File(" + filePath.string() + "). Check for permissions and locks.");
    }

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

bool StorageManager::fileExists(std::filesystem::path filePath)
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

        config_file.seekp(0, std::ios::beg);

        config_file << json_config_file.dump(4);

        config_file.flush();

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: Failed to dump commands to disk. Details: " << e.what() << "\n";
        return false;
    }
}