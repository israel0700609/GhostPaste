#pragma once

#include <string>
#include <optional>
#include <cstddef>

class TextInjector
{
public:
    static void replaceText(size_t eraseCount, const std::string &replacementText);

private:
    static void eraseCharacters(size_t count);
    static void sendCtrlV();
    static std::optional<std::wstring> getClipboardText();
    static bool setClipboardText(const std::wstring &text);
    static std::wstring utf8ToWide(const std::string &utf8Str);
};