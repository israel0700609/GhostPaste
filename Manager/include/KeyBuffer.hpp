#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <windows.h>
#include "MacroManager.hpp"
#include "KeyboardHook.hpp"

struct MatchResult
{
    size_t triggerLength;
    std::string replacementValue;
};

class KeyBuffer
{
public:
    explicit KeyBuffer(MacroManager &macroManager,
                       size_t maxBufferSize = 64,
                       std::chrono::milliseconds typingTimeout = std::chrono::milliseconds(2000));

    std::optional<MatchResult> processEvent(KeyEventType type, char ch);
    void clear();

private:
    MacroManager &macroManager;
    std::string buffer;
    size_t maxBufferSize;
    std::chrono::milliseconds typingTimeout;

    HWND lastWindow = nullptr;
    std::chrono::steady_clock::time_point m_lastKeyTime;
};