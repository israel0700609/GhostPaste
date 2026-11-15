#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "MacroManager.hpp"
#include "StorageManager.hpp"

class AutoSaver
{
public:
    AutoSaver(MacroManager &macroManager,
              StorageManager &storageManager,
              std::chrono::seconds interval = std::chrono::seconds(30));
    ~AutoSaver();

    AutoSaver(const AutoSaver &) = delete;
    AutoSaver &operator=(const AutoSaver &) = delete;

    void start();
    void stop();

private:
    void workerLoop();

    MacroManager &macroManager;
    StorageManager &storageManager;
    std::chrono::seconds interval;

    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool stopRequested = false;
};