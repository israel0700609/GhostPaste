#include "Command.hpp"

Command::Command() : trigger(""), value(""), description(""),
                     is_case_sensitive(false), enabled(true), usage_count(0) {}

Command::Command(const std::string &trigger, const std::string &value,
                 const std::string &description,
                 bool is_case_sensitive,
                 bool enabled,
                 int usage_count)
    : trigger(trigger), value(value), description(description),
      is_case_sensitive(is_case_sensitive), enabled(enabled), usage_count(usage_count)
{
}

std::string Command::getTrigger() const { return trigger; }
std::string Command::getValue() const { return value; }
std::string Command::getDescription() const { return description; }
bool Command::isCaseSensitive() const { return is_case_sensitive; }
bool Command::isEnabled() const { return enabled; }
int Command::getUsageCount() const { return usage_count; }

void Command::setTrigger(const std::string &trigger) { this->trigger = trigger; }
void Command::setValue(const std::string &value) { this->value = value; }
void Command::setDescription(const std::string &description) { this->description = description; }
void Command::setCaseSensitive(bool case_sensitive) { this->is_case_sensitive = case_sensitive; }
void Command::setEnabled(bool enabled) { this->enabled = enabled; }
void Command::setUsageCount(int usage_count) { this->usage_count = usage_count; }
void Command::incrementUsageCount() { this->usage_count += 1; }
