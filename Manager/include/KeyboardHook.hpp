#pragma once

#include <windows.h>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

enum class KeyEventType
{
    CHARACTER,
    BACKSPACE,
    RESET
};

class KeyboardHook
{
public:
    using KeyCallback = std::function<void(KeyEventType type, char ch)>;

    KeyboardHook();
    ~KeyboardHook();

    KeyboardHook(const KeyboardHook &) = delete;
    KeyboardHook &operator=(const KeyboardHook &) = delete;

    void setCallback(KeyCallback callback);
    void start();
    void stop();
    void eraseCharacters(size_t count);

private:
    void hookThreadLoop();
    static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static void translateAndDispatch(const KBDLLHOOKSTRUCT *kbdStruct);

    static KeyboardHook *instance;

    HHOOK hookHandle = nullptr;
    std::thread thread;
    std::atomic<bool> running{false};
    std::atomic<DWORD> threadId{0};
    KeyCallback callback;
};