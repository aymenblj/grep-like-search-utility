# Grep-Like Search Utility

A multi-threaded, cross-platform C++ utility for recursively searching through directories for text patterns, with optional case sensitivity, regular expression support, and result highlighting similar to `grep`.

## Features

- Recursive directory search
- Multi-threaded file processing
- Case-sensitive or case-insensitive matching
- Optional regular expression support
- Highlighting of matched patterns in output
- Unit-tested and modular design
- Portable: Windows, Linux, and macOS compatible
- Doxygen configured and documented

## Build Instructions

### Requirements

- CMake 3.16+
- C++20-compatible compiler (GCC, Clang, MSVC)
- Ninja (optional but recommended for faster builds)
- Gradle (optional)
- Doxygen (optional)

### Post Build: Install Optional Tools

These tools are optional but recommended for full development workflow (faster builds, documentation, extra Gradle-based tasks).

<details>
<summary><strong> Ubuntu / Debian</strong></summary>

```bash
sudo apt-get update
sudo apt-get install ninja-build doxygen gradle
```

</details> <details> <summary><strong> macOS (with Homebrew)</strong></summary>

```bash
brew update
brew install ninja doxygen gradle
```

</details> <details> <summary><strong> Windows (with Chocolatey)</strong></summary>
Run Command Prompt or PowerShell as Administrator:

```bash
choco install ninja doxygen gradle
```
</details>

### Build

```bash
mkdir build && cd build
cmake -G"Ninja" .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake
ninja
```

### Run unit tests

```bash
ninja runUnitTests
```

### Alternatively, run the Gradle Tasks (optional)

```bash
# Build the project
gradle build

# Clean main build
gradle clean

# Clean everything
gradle cleanAll

# Run unit tests
gradle runTests

```

## Usage

```bash
build/src/FileSearcher <directory> <query> [--ignore-case] [--regex]
```

### Examples

```bash
# Simple case-sensitive search
build/src/FileSearcher ./examples "Paris"

# Case-sensitive search
build/src/FileSearcher ./examples "architecture"

# Case-insensitive search
build/src/FileSearcher ./examples "architecture" --ignore-case

# Regex search with highlighting
build/src/FileSearcher ./examples "tec." --regex
build/src/FileSearcher examples "tec.*" --ignore-case --regex
```

### Design Overview

- FileSearcher: Interface for file searching.

- TextFileSearcher: Implements the search logic using regex or plain search.

- SearchManager: Manages file distribution and threading.

- HighlightMatches: Highlights matches inline using ANSI colors.

- Modular Design with CMake for build Structure and Dependency handling
