#pragma once
#include <string>
#include <iostream>

enum class CommandType
{
    ADD,
    UPDATE,
    REMOVE,
    LIST,
    INFO,
    ENABLE,
    DISABLE,
    CASE_SENSITIVE,
    TIMEOUT,
    UNTIMEOUT,
    RELOAD,
    EXPORT,
    IMPORT,
    CLEAR,
    HELP,
    EXIT,
    UNKNOWN
};
class Command
{
private:
    std::string trigger;
    std::string value;
    std::string description;
    bool is_case_sensitive;
    bool enabled;
    int usage_count;

public:
    Command();
    Command(const std::string &trigger, const std::string &value,
            const std::string &description = "",
            bool is_case_sensitive = false,
            bool enabled = true,
            int usage_count = 0);

    std::string getTrigger() const;
    std::string getValue() const;
    std::string getDescription() const;
    bool isCaseSensitive() const;
    bool isEnabled() const;
    int getUsageCount() const;

    void setTrigger(const std::string &trigger);
    void setValue(const std::string &value);
    void setDescription(const std::string &description);
    void setCaseSensitive(bool case_sensitive);
    void setEnabled(bool enabled);
    void setUsageCount(int usage_count);
};
