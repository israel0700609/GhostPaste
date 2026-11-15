#include "KeyBuffer.hpp"

KeyBuffer::KeyBuffer(MacroManager &macroManager, size_t maxSize)
    : macroManager(macroManager), maxSize(maxSize)
{
}

void KeyBuffer::clear()
{
    buffer.clear();
}

std::optional<MatchResult> KeyBuffer::processEvent(KeyEventType type, char ch)
{
    if (type == KeyEventType::RESET)
    {
        clear();
        return std::nullopt;
    }

    if (type == KeyEventType::BACKSPACE)
    {
        if (!buffer.empty())
        {
            buffer.pop_back();
        }

        return std::nullopt;
    }

    buffer.push_back(ch);
    if (buffer.size() > maxSize)
    {
        buffer.erase(0, buffer.size() - maxSize);
    }

    auto commands = macroManager.getCommandsSnapshot();

    for (const auto &[trigger, cmd] : commands)
    {
        if (!cmd.isEnabled())
        {
            continue;
        }

        if (Util::endsWith(buffer, trigger, cmd.isCaseSensitive()))
        {
            macroManager.incrementCommandUsage(trigger);
            clear();
            return MatchResult{trigger.size(), cmd.getValue()};
        }
    }

    return std::nullopt;
}