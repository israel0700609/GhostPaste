#include <iostream>
#include <exception>
#include <windows.h>
#include "StorageManager.hpp"
#include "MacroManager.hpp"
#include "CommandLoop.hpp"
#include "KeyboardHook.hpp"
#include "KeyBuffer.hpp"
#include "TextInjector.hpp"
#include "Utils.hpp"
#include "AutoSaver.hpp"

namespace
{
    MacroManager *g_macroManager = nullptr;
    StorageManager *g_storage = nullptr;
    KeyboardHook *g_hook = nullptr;
    AutoSaver *g_autoSaver = nullptr;

    BOOL WINAPI consoleCtrlHandler(DWORD ctrlType)
    {
        switch (ctrlType)
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            if (g_hook)
            {
                g_hook->stop();
            }
            if (g_autoSaver)
            {
                g_autoSaver->stop();
            }
            if (g_macroManager && g_storage)
            {
                Util::saveChangesToDisk(*g_macroManager, *g_storage);
            }
            return FALSE;
        default:
            return FALSE;
        }
    }
}

int main()
{
    HANDLE hSingleInstanceMutex = CreateMutexA(nullptr, TRUE, "Global\\GhostPaste_SingleInstance_Mutex");
    if (hSingleInstanceMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        std::cerr << "GhostPaste is already running in another window!\n";
        if (hSingleInstanceMutex)
        {
            CloseHandle(hSingleInstanceMutex);
        }
        return 1;
    }

    try
    {
        StorageManager storage("data/config.json");
        auto initialCommands = storage.loadCommands();

        MacroManager macroManager(initialCommands);
        KeyBuffer keyBuffer(macroManager);
        KeyboardHook hook;
        AutoSaver autoSaver(macroManager, storage, std::chrono::seconds(30));

        g_macroManager = &macroManager;
        g_storage = &storage;
        g_hook = &hook;
        g_autoSaver = &autoSaver;
        SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

        autoSaver.start();

        hook.setCallback([&keyBuffer](KeyEventType type, char ch) -> bool
                         {
            HWND foreground = GetForegroundWindow();
            if (foreground == GetConsoleWindow())
            {
                keyBuffer.clear();
                return false;
            }

            char windowTitle[256] = {0};
            if (GetWindowTextA(foreground, windowTitle, sizeof(windowTitle)) > 0)
            {
                if (std::string(windowTitle).find("GhostPaste") != std::string::npos)
                {
                    keyBuffer.clear();
                    return false;
                }
            }

            auto match = keyBuffer.processEvent(type, ch);
            if (match.has_value())
            {
                TextInjector::replaceText(match->triggerLength - 1, match->replacementValue);
                return true;
            }

            return false; });

        SetConsoleTitleA("GhostPaste CLI");
        hook.start();

        std::cout << "Welcome to GhostPaste! Type /help for available commands.\n\n";
        CommandLoop loop(macroManager, storage);
        loop.start();

        SetConsoleCtrlHandler(consoleCtrlHandler, FALSE);
        g_macroManager = nullptr;
        g_storage = nullptr;
        g_hook = nullptr;
        g_autoSaver = nullptr;

        hook.stop();
        autoSaver.stop();

        Util::saveChangesToDisk(macroManager, storage);
        std::cout << "Goodbye!\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        ReleaseMutex(hSingleInstanceMutex);
        CloseHandle(hSingleInstanceMutex);
        return 1;
    }

    ReleaseMutex(hSingleInstanceMutex);
    CloseHandle(hSingleInstanceMutex);
    return 0;
}