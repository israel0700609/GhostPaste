#include "AutoSaver.hpp"
#include "Utils.hpp"

AutoSaver::AutoSaver(MacroManager &macroManager,
                     StorageManager &storageManager,
                     std::chrono::seconds interval)
    : macroManager(macroManager),
      storageManager(storageManager),
      interval(interval)
{
}

AutoSaver::~AutoSaver()
{
    stop();
}

void AutoSaver::start()
{
    if (thread.joinable())
    {
        return;
    }

    stopRequested = false;
    thread = std::thread(&AutoSaver::workerLoop, this);
}

void AutoSaver::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (stopRequested && !thread.joinable())
        {
            return;
        }
        stopRequested = true;
    }

    cv.notify_one();

    if (thread.joinable())
    {
        thread.join();
    }
}

void AutoSaver::workerLoop()
{
    std::unique_lock<std::mutex> lock(mutex);

    while (!stopRequested)
    {
        cv.wait_for(lock, interval, [this]()
                    { return stopRequested; });

        if (stopRequested)
        {
            break;
        }

        lock.unlock();
        Util::saveChangesToDisk(macroManager, storageManager);
        lock.lock();
    }
}