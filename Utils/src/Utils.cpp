
#include "Utils.hpp"

Command Util::deserialize(nlohmann::json &jsonCommand)
{
    std::string trigger = jsonCommand.value("trigger", "undifined");
    std::string value = jsonCommand.value("value", "unknown");
    std::string description = jsonCommand.value("description", " ");
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
