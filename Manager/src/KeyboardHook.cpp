#include "KeyboardHook.hpp"
#include <iostream>

KeyboardHook *KeyboardHook::instance = nullptr;

KeyboardHook::KeyboardHook()
{
    instance = this;
}

KeyboardHook::~KeyboardHook()
{
    stop();
    if (instance == this)
    {
        instance = nullptr;
    }
}

void KeyboardHook::setCallback(KeyCallback callback)
{
    this->callback = callback;
}

void KeyboardHook::start()
{
    if (running.load())
    {
        return;
    }

    running.store(true);
    thread = std::thread(&KeyboardHook::hookThreadLoop, this);
}

void KeyboardHook::stop()
{
    if (!running.load())
    {
        return;
    }

    running.store(false);

    if (threadId.load() != 0)
    {
        PostThreadMessage(threadId, WM_QUIT, 0, 0);
    }

    if (thread.joinable())
    {
        thread.join();
    }
}

void KeyboardHook::hookThreadLoop()
{
    threadId.store(GetCurrentThreadId());

    hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, lowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    if (!hookHandle)
    {
        std::cerr << "Fatal Error: Failed to install keyboard hook. Error code: " << GetLastError() << "\n";
        stop();
        return;
    }

    MSG msg;
    while (running.load() && GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hookHandle);
    hookHandle = nullptr;
    threadId.store(0);
}

LRESULT CALLBACK KeyboardHook::lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        auto *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        if ((kbdStruct->flags & LLKHF_INJECTED) == 0 && instance && instance->callback)
        {
            if (translateAndDispatch(kbdStruct))
            {
                return 1;
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
bool KeyboardHook::translateAndDispatch(const KBDLLHOOKSTRUCT *kbdStruct)
{
    DWORD vkCode = kbdStruct->vkCode;

    if (vkCode == VK_BACK)
    {
        instance->callback(KeyEventType::BACKSPACE, '\0');
        return false;
    }

    if (vkCode == VK_RETURN || vkCode == VK_TAB || vkCode == VK_ESCAPE ||
        (vkCode >= VK_LEFT && vkCode <= VK_DOWN))
    {
        instance->callback(KeyEventType::RESET, '\0');
        return false;
    }

    bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    bool altPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    if (ctrlPressed || altPressed)
    {
        instance->callback(KeyEventType::RESET, '\0');
        return false;
    }

    BYTE keyboardState[256] = {0};
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
    {
        keyboardState[VK_SHIFT] = 0x80;
    }
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
    {
        keyboardState[VK_CAPITAL] = 0x01;
    }

    wchar_t unicodeBuffer[4] = {0};
    int result = ToUnicode(vkCode, kbdStruct->scanCode, keyboardState, unicodeBuffer, 4, 0);

    if (result == 1 && unicodeBuffer[0] >= 32 && unicodeBuffer[0] <= 126)
    {
        return instance->callback(KeyEventType::CHARACTER, static_cast<char>(unicodeBuffer[0]));
    }

    return false;
}
