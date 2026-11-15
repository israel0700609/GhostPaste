#include "KeyBuffer.hpp"
#include "Utils.hpp"

KeyBuffer::KeyBuffer(MacroManager &macroManager,
                     size_t maxBufferSize,
                     std::chrono::milliseconds typingTimeout)
    : macroManager(macroManager),
      maxBufferSize(maxBufferSize),
      typingTimeout(typingTimeout),
      m_lastKeyTime(std::chrono::steady_clock::now())
{
}

void KeyBuffer::clear()
{
    buffer.clear();
}

std::optional<MatchResult> KeyBuffer::processEvent(KeyEventType type, char ch)
{
    auto now = std::chrono::steady_clock::now();
    HWND currentWindow = GetForegroundWindow();

    if (currentWindow != lastWindow || (now - m_lastKeyTime) > typingTimeout)
    {
        clear();
    }

    lastWindow = currentWindow;
    m_lastKeyTime = now;

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
    if (buffer.size() > maxBufferSize)
    {
        buffer.erase(0, buffer.size() - maxBufferSize);
    }

    auto commands = macroManager.getCommandsSnapshot();
    for (const auto &[trigger, cmd] : commands)
    {
        if (!cmd.isEnabled() || trigger.empty())
        {
            continue;
        }

        if (Util::endsWith(buffer, trigger, cmd.isCaseSensitive()))
        {
            std::string replacement = cmd.getValue();
            size_t triggerLen = trigger.size();

            macroManager.incrementCommandUsage(trigger);
            clear();

            return MatchResult{triggerLen, std::move(replacement)};
        }
    }

    return std::nullopt;
}