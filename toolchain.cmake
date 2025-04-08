cmake_minimum_required(VERSION 3.15)

# Detect the OS and find the appropriate compiler
if(WIN32)
    message(STATUS "Configuring for Windows")

    # Try to find g++ in PATH
    find_program(GXX_EXECUTABLE NAMES g++ g++.exe)
    
    if(NOT GXX_EXECUTABLE)
        message(FATAL_ERROR "g++ not found! Please install MinGW-w64 and add it to your PATH.")
    endif()

    set(CMAKE_CXX_COMPILER "${GXX_EXECUTABLE}")
    message(STATUS "Using C++ Compiler: ${CMAKE_CXX_COMPILER}")

    add_compile_definitions(_WIN32)
    add_compile_options(-Wall -Wextra -Wpedantic)

elseif(APPLE)
    message(STATUS "Configuring for macOS")

    # Prefer clang++ but fall back to g++
    find_program(CLANG_EXECUTABLE NAMES clang++)

    if(CLANG_EXECUTABLE)
        set(CMAKE_CXX_COMPILER "${CLANG_EXECUTABLE}")
    else()
        find_program(GXX_EXECUTABLE NAMES g++)
        if(NOT GXX_EXECUTABLE)
            message(FATAL_ERROR "Neither clang++ nor g++ found! Install Xcode command line tools.")
        endif()
        set(CMAKE_CXX_COMPILER "${GXX_EXECUTABLE}")
    endif()

    message(STATUS "Using C++ Compiler: ${CMAKE_CXX_COMPILER}")
    add_compile_options(-Wall -Wextra -Wpedantic)

elseif(UNIX)
    message(STATUS "Configuring for Linux")

    # Try to find g++ in PATH
    find_program(GXX_EXECUTABLE NAMES g++)

    if(NOT GXX_EXECUTABLE)
        message(FATAL_ERROR "g++ not found! Install GCC with your package manager (e.g., apt, yum, pacman).")
    endif()

    set(CMAKE_CXX_COMPILER "${GXX_EXECUTABLE}")
    message(STATUS "Using C++ Compiler: ${CMAKE_CXX_COMPILER}")

    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
