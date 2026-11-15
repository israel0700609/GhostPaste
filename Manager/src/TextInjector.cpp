#include "TextInjector.hpp"
#include <windows.h>
#include <vector>
#include <thread>
#include <cstring>

void TextInjector::replaceText(size_t eraseCount, const std::string &replacementText)
{
    eraseCharacters(eraseCount);

    std::optional<std::wstring> previousClipboard = getClipboardText();

    std::wstring wideText = utf8ToWide(replacementText);
    if (!setClipboardText(wideText))
    {
        return;
    }

    sendCtrlV();

    if (previousClipboard.has_value())
    {
        std::wstring backup = std::move(*previousClipboard);
        std::thread([backup = std::move(backup)]()
                    {
            Sleep(60); 
            setClipboardText(backup); })
            .detach();
    }
}

void TextInjector::eraseCharacters(size_t count)
{
    if (count == 0)
    {
        return;
    }

    std::vector<INPUT> inputs(count * 2, INPUT{});
    for (size_t i = 0; i < count; ++i)
    {
        inputs[i * 2].type = INPUT_KEYBOARD;
        inputs[i * 2].ki.wVk = VK_BACK;

        inputs[i * 2 + 1].type = INPUT_KEYBOARD;
        inputs[i * 2 + 1].ki.wVk = VK_BACK;
        inputs[i * 2 + 1].ki.dwFlags = KEYEVENTF_KEYUP;
    }

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void TextInjector::sendCtrlV()
{
    INPUT inputs[4] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

std::optional<std::wstring> TextInjector::getClipboardText()
{
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(nullptr))
    {
        return std::nullopt;
    }

    std::optional<std::wstring> result = std::nullopt;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData != nullptr)
    {
        auto *pszText = static_cast<const wchar_t *>(GlobalLock(hData));
        if (pszText != nullptr)
        {
            result = std::wstring(pszText);
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result;
}

bool TextInjector::setClipboardText(const std::wstring &text)
{
    if (!OpenClipboard(nullptr))
    {
        return false;
    }

    EmptyClipboard();

    size_t sizeInBytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, sizeInBytes);
    if (!hGlob)
    {
        CloseClipboard();
        return false;
    }

    void *pDest = GlobalLock(hGlob);
    if (!pDest)
    {
        GlobalFree(hGlob);
        CloseClipboard();
        return false;
    }

    std::memcpy(pDest, text.c_str(), sizeInBytes);
    GlobalUnlock(hGlob);

    if (!SetClipboardData(CF_UNICODETEXT, hGlob))
    {
        GlobalFree(hGlob);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

std::wstring TextInjector::utf8ToWide(const std::string &utf8Str)
{
    if (utf8Str.empty())
    {
        return L"";
    }

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), nullptr, 0);
    std::wstring wideStr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), wideStr.data(), sizeNeeded);
    return wideStr;
}