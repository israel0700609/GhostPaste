#pragma once

#include <string>
#include <optional>
#include "KeyboardHook.hpp"
#include "MacroManager.hpp"
#include "Utils.hpp"

struct MatchResult
{
    size_t triggerLength;
    std::string replacementValue;
};

class KeyBuffer
{
public:
    explicit KeyBuffer(MacroManager &macroManager, size_t maxSize = 64);

    std::optional<MatchResult> processEvent(KeyEventType type, char ch);
    void clear();

private:
    MacroManager &macroManager;
    std::string buffer;
    size_t maxSize;
};