#pragma once

#include <windows.h>
#include <functional>
#include <thread>
#include <atomic>

enum class KeyEventType
{
    CHARACTER,
    BACKSPACE,
    RESET
};

class KeyboardHook
{
public:
    using KeyCallback = std::function<bool(KeyEventType type, char ch)>;
    KeyboardHook();
    ~KeyboardHook();

    KeyboardHook(const KeyboardHook &) = delete;
    KeyboardHook &operator=(const KeyboardHook &) = delete;

    void setCallback(KeyCallback callback);
    void start();
    void stop();

private:
    void hookThreadLoop();
    static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static bool translateAndDispatch(const KBDLLHOOKSTRUCT *kbdStruct);

    static KeyboardHook *instance;

    HHOOK hookHandle = nullptr;
    std::thread thread;
    std::atomic<bool> running{false};
    std::atomic<DWORD> threadId{0};
    KeyCallback callback;
};