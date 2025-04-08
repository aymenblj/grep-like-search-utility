# =====================================================================================================
# Modular Design with CMake for Build Structure and Dependency Handling
# =====================================================================================================
# - Source and dependencies are organized into modular subdirectories.
# - External dependencies are handled via FetchContent and file config in a separate 'dependencies' directory.
# - Top-level CMakeLists.txt orchestrates the entire build process for scalability and maintainability.
#
# This approach makes the project scalable and maintainable, where each module can evolve independently
# without affecting other parts of the system. Dependencies are handled in a centralized manner for 
# consistency and easier upgrades.

cmake_minimum_required(VERSION 3.16)

project(FileSearcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# include project dependencies/Third-parties
include(dependencies/testing.cmake)

# Add source and include directories
include_directories(include)

# include src
add_subdirectory(src)

# Add an executable for unit tests
enable_testing()
add_subdirectory(unitTests)

include(dependencies/doxygen.cmake)

# Call the function (no arguments needed for minimalist)
add_doxygen_target()
