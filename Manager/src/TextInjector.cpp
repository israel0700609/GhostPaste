#include "TextInjector.hpp"
#include <windows.h>
#include <vector>
#include <thread>
#include <cstring>

void TextInjector::replaceText(size_t eraseCount, const std::string &replacementText)
{
    std::thread([eraseCount, replacementText]()
                {
        eraseCharacters(eraseCount);

        std::optional<std::wstring> previousClipboard = getClipboardText();

        std::wstring wideText = utf8ToWide(replacementText);
        if (!setClipboardText(wideText))
        {
            return;
        }

        Sleep(15);

        sendCtrlV();

        if (previousClipboard.has_value())
        {
            Sleep(150);
            setClipboardText(*previousClipboard);
        } })
        .detach();
}

void TextInjector::eraseCharacters(size_t count)
{
    if (count == 0)
    {
        return;
    }

    WORD backScan = static_cast<WORD>(MapVirtualKeyA(VK_BACK, MAPVK_VK_TO_VSC));
    std::vector<INPUT> inputs(count * 2, INPUT{});

    for (size_t i = 0; i < count; ++i)
    {
        inputs[i * 2].type = INPUT_KEYBOARD;
        inputs[i * 2].ki.wVk = VK_BACK;
        inputs[i * 2].ki.wScan = backScan;

        inputs[i * 2 + 1].type = INPUT_KEYBOARD;
        inputs[i * 2 + 1].ki.wVk = VK_BACK;
        inputs[i * 2 + 1].ki.wScan = backScan;
        inputs[i * 2 + 1].ki.dwFlags = KEYEVENTF_KEYUP;
    }

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void TextInjector::sendCtrlV()
{
    WORD ctrlScan = static_cast<WORD>(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC));
    WORD vScan = static_cast<WORD>(MapVirtualKeyA('V', MAPVK_VK_TO_VSC));

    std::vector<INPUT> inputs;
    inputs.reserve(8);

    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
    {
        INPUT shiftUp = {};
        shiftUp.type = INPUT_KEYBOARD;
        shiftUp.ki.wVk = VK_SHIFT;
        shiftUp.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(shiftUp);
    }
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0)
    {
        INPUT altUp = {};
        altUp.type = INPUT_KEYBOARD;
        altUp.ki.wVk = VK_MENU;
        altUp.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(altUp);
    }

    INPUT ctrlDown = {};
    ctrlDown.type = INPUT_KEYBOARD;
    ctrlDown.ki.wVk = VK_CONTROL;
    ctrlDown.ki.wScan = ctrlScan;
    inputs.push_back(ctrlDown);

    INPUT vDown = {};
    vDown.type = INPUT_KEYBOARD;
    vDown.ki.wVk = 'V';
    vDown.ki.wScan = vScan;
    inputs.push_back(vDown);

    INPUT vUp = {};
    vUp.type = INPUT_KEYBOARD;
    vUp.ki.wVk = 'V';
    vUp.ki.wScan = vScan;
    vUp.ki.dwFlags = KEYEVENTF_KEYUP;
    inputs.push_back(vUp);

    INPUT ctrlUp = {};
    ctrlUp.type = INPUT_KEYBOARD;
    ctrlUp.ki.wVk = VK_CONTROL;
    ctrlUp.ki.wScan = ctrlScan;
    ctrlUp.ki.dwFlags = KEYEVENTF_KEYUP;
    inputs.push_back(ctrlUp);

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

std::optional<std::wstring> TextInjector::getClipboardText()
{
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        return std::nullopt;
    }

    bool opened = false;
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        if (OpenClipboard(nullptr))
        {
            opened = true;
            break;
        }
        Sleep(5);
    }

    if (!opened)
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
    bool opened = false;
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        if (OpenClipboard(nullptr))
        {
            opened = true;
            break;
        }
        Sleep(5);
    }

    if (!opened)
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