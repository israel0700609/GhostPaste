# GhostPaste

**GhostPaste** is a lightweight, high-performance Windows text expansion engine written in modern C++17. It runs silently in the background, intercepts global keyboard input at the OS level, and expands custom text triggers into full snippets in real time-while preserving your clipboard and system responsiveness.

---

## Key Features

- **System-Wide Keystroke Interception:** Uses a low-level Windows keyboard hook (`WH_KEYBOARD_LL`) to detect triggers across any active application (browsers, IDEs, messaging apps, editors).
- **Zero-Flicker Replacement & Clipboard Preservation:** Automatically erases the typed trigger, backs up your existing clipboard content, injects the snippet via hardware scan-code `Ctrl+V`, and restores your original clipboard.
- **Modifier & Layout Resilient:** Safely releases held `Shift`/`Alt` keys prior to pasting (preventing accidental `Ctrl+Shift+V`) and uses hardware scan codes (`MapVirtualKeyA`) so shortcuts work reliably regardless of active keyboard layout.
- **Smart Context Reset:** Automatically clears the rolling input buffer when switching between windows, pressing navigation keys, or pausing typing for more than 2 seconds.
- **Self-Exclusion Guard:** Detects when the GhostPaste CLI console is in focus and suppresses macro expansion while you manage commands.
- **Thread-Safe Architecture & Auto-Persistence:** Separates real-time hook processing, CLI management, and background disk auto-saving (`AutoSaver`) using `std::mutex` and `std::condition_variable`.
- **Crash & Exit Protection:** Enforces single-instance execution via a named Windows Mutex and hooks `SetConsoleCtrlHandler` to flush unsaved changes to `config.json` even if the terminal window is closed abruptly.

---

## Architecture Overview

GhostPaste is structured into decoupled modules across a multi-threaded pipeline:

```text
[Main Thread: CLI]                         [Background Thread: OS Hook]
==================                         ============================
  CommandLoop (User Input)                   Windows OS (Physical Keystroke)
       │                                              │
       ▼                                              ▼
  MacroManager ◄── (std::mutex protected) ──►   KeyboardHook (WH_KEYBOARD_LL)
  (Shared State)                                      │
       │                                              ▼
       ▼                                         KeyBuffer (Rolling Match)
  AutoSaver / StorageManager                          │
  (data/config.json)                             [Match Found?] ──(No)──► Pass Through
                                                      │
                                                    (Yes)
                                                      ▼
                                                 TextInjector (Async Thread)
                                                 (Erase -> Backup -> Paste -> Restore)
```

### Core Components

| Component | Responsibility |
| :--- | :--- |
| **`KeyboardHook`** | Installs and manages the `WH_KEYBOARD_LL` hook, filters out injected events (`LLKHF_INJECTED`), translates virtual key codes, and swallows the final trigger character (`return 1`). |
| **`KeyBuffer`** | Maintains a bounded rolling buffer of recent characters. Resets on window focus changes (`GetForegroundWindow`), typing timeouts (2s), or control keys, and matches active triggers using `std::string_view`. |
| **`TextInjector`** | Executes text replacement asynchronously on a detached worker thread: sends `VK_BACK` scan codes, safely locks/backs up `CF_UNICODETEXT` clipboard data with retry logic, dispatches `Ctrl+V`, and restores the previous clipboard state. |
| **`MacroManager`** | Thread-safe central repository (`std::mutex`) storing macro definitions, enabled/disabled states, usage counters, dirty flags, and temporary global pause timers. |
| **`StorageManager`** | Handles serialization and deserialization of macros to and from `data/config.json` using `nlohmann::json`. |
| **`AutoSaver`** | Background persistence worker driven by `std::condition_variable::wait_for`. Periodically flushes dirty state to disk (every 30s) without CPU polling and wakes immediately on shutdown. |
| **`CommandLoop`** | Interactive terminal interface for creating, toggling, inspecting, and saving macros at runtime. |

---

## Project Structure

```text
GhostPaste/
├── CMakeLists.txt
├── main.cpp                  # Entry point, Single-Instance Mutex, ConsoleCtrlHandler, wiring
├── data/
│   └── config.json           # Persistent macro storage (auto-generated/updated)
├── Model/
│   ├── include/Command.hpp
│   └── src/Command.cpp
├── Manager/
│   ├── include/
│   │   ├── AutoSaver.hpp
│   │   ├── CommandLoop.hpp
│   │   ├── KeyBuffer.hpp
│   │   ├── KeyboardHook.hpp
│   │   ├── MacroManager.hpp
│   │   ├── StorageManager.hpp
│   │   └── TextInjector.hpp
│   └── src/
│       └── ...
└── Utils/
    ├── include/Utils.hpp
    └── src/Utils.cpp
```

---

## Prerequisites & Building with CMake

### Requirements
- **OS:** Windows 10 / Windows 11
- **Compiler:** C++17 compatible compiler (MinGW-w64 GCC or MSVC)
- **Build System:** CMake 3.15+
- **Dependencies:** [`nlohmann-json`](https://github.com/nlohmann/json) (recommended via `vcpkg`)

### Build Steps (with vcpkg)

```powershell
# 1. Clone the repository
git clone [https://github.com/israel0700609/GhostPaste.git](https://github.com/israel0700609/GhostPaste.git)
cd GhostPaste

# 2. Configure the build directory with CMake
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# 3. Compile the executable
cmake --build .
```

### Running the Application

```powershell
./GhostPaste.exe
```

> **Note:** On Windows, User Interface Privilege Isolation (UIPI) prevents standard processes from injecting keystrokes into elevated windows. If you want GhostPaste to expand text inside applications running **As Administrator** (such as Task Manager or an elevated terminal), run `GhostPaste.exe` as Administrator as well.

---

---

## CLI Usage

Once running, GhostPaste listens globally in the background while keeping an interactive management prompt open in its terminal window. Most commands support passing arguments inline or entering them interactively if omitted.

### Macro Management

| Command | Description | Example |
| :--- | :--- | :--- |
| `/add <trigger>` | Create a new macro (prompts interactively for `value`, optional `description`, and `is_case_sensitive`). | `/add !mail` |
| `/update <trigger>` | Edit an existing macro's `value`, `description`, case sensitivity, or enabled status (press `Enter` to keep current values). | `/update !mail` |
| `/remove <trigger>` | Delete a macro after confirmation (`y/n`). | `/remove !mail` |
| `/list` | Display all saved macros along with their details, status, and usage counts. | `/list` |
| `/info <trigger>` | Show full details and usage statistics for a specific macro. | `/info !mail` |

### State & Behavior

| Command | Description | Example |
| :--- | :--- | :--- |
| `/enable <trigger>` | Activate a disabled macro. | `/enable !mail` |
| `/disable <trigger>` | Temporarily turn off a macro without deleting it. | `/disable !mail` |
| `/casensitive <trigger>` | Toggle case sensitivity (`true`/`false`) for a specific macro. | `/casensitive !mail` |
| `/timeout <trigger>` | Temporarily pause a specific macro for a given number of minutes (prompts for duration). | `/timeout !mail` |
| `/untimeout <trigger>` | Cancel an active timeout on a macro immediately. | `/untimeout !mail` |

### Data & Backup

| Command | Description | Example |
| :--- | :--- | :--- |
| `/reload` | Reload macros from `data/config.json` after confirmation (`y/n`). | `/reload` |
| `/export <path>` | Export all saved macros to an external JSON backup file. | `/export backups/macros.json` |
| `/import <path>` | Import macros from an external JSON file. | `/import backups/macros.json` |
| `/clear` | Delete all saved macros after confirmation (`y/n`). | `/clear` |

### General

| Command | Description | Example |
| :--- | :--- | :--- |
| `/help` | Display the built-in command guide. | `/help` |
| `/exit` | Stop background threads, flush changes to disk, and exit cleanly. | `/exit` |

---

## Configuration Format (`data/config.json`)

Macros are stored in human-readable JSON format and loaded automatically on startup:

```json
{
    "!email": {
        "description": "Main Gmail",
        "enabled": true,
        "is_case_sensitive": false,
        "trigger": "!email",
        "usage_count": 14,
        "value": "contact@exmaple.com"
    }
}
```
