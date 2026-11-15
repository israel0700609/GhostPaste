cmake_minimum_required(VERSION 3.14)

set(VCPKG_TARGET_TRIPLET x64-mingw-dynamic CACHE STRING "")

project(GhostPaste VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

add_executable(GhostPaste
    main.cpp
    Model/src/Command.cpp
    Manager/src/MacroManager.cpp
    Manager/src/StorageManager.cpp
    Utils/src/Utils.cpp
    Manager/src/CommandLoop.cpp
    Manager/src/KeyboardHook.cpp
)

target_link_libraries(GhostPaste PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(GhostPaste PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Model/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Model/include    
    ${CMAKE_CURRENT_SOURCE_DIR}/Manager/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Manager/include  
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/include   
)

target_compile_definitions(GhostPaste PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")