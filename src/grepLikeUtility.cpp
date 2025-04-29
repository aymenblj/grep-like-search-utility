#include "grepLikeUtility.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <mutex>
#include <locale>
#include <regex>

// ANSI escape codes for coloring text in the terminal
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

// Define a static mutex for safe concurrent console output
std::mutex TextFileSearcher::coutMutex;

/**
 * @brief Highlights matching substrings in a given line.
 * 
 * This function adds ANSI color codes around the matched query in a line.
 * It works in both plain string mode and regex mode, and it can be configured
 * to ignore case sensitivity.
 *
 * @param line The line of text to search and highlight.
 * @param query The string or regex pattern to search for.
 * @param caseSensitive Whether matching should be case-sensitive.
 * @param useRegex Whether to interpret the query as a regular expression.
 * @return A new string with matching parts wrapped in color codes.
 */
std::string highlightMatches(const std::string& line, const std::string& query, bool caseSensitive, bool useRegex) {
    std::string result;

    if (!useRegex) {
        const std::string& searchBase = caseSensitive ? line : [&] {
            static thread_local std::string lowered;
            lowered = line;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
            return lowered;
        }();

        std::string loweredQuery = query;
        if (!caseSensitive) {
            std::transform(loweredQuery.begin(), loweredQuery.end(), loweredQuery.begin(), ::tolower);
        }

        size_t pos = 0;
        while (true) {
            size_t found = searchBase.find(loweredQuery, pos);
            if (found == std::string::npos) {
                result += line.substr(pos);  // append remaining unmatched
                break;
            }

            result += line.substr(pos, found - pos); // unmatched part
            result += COLOR_YELLOW + line.substr(found, query.length()) + COLOR_RESET;
            pos = found + query.length();
        }

    } else {
        std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
        if (!caseSensitive) flags |= std::regex::icase;

        try {
            std::regex pattern(query, flags);
            std::sregex_iterator it(line.begin(), line.end(), pattern);
            std::sregex_iterator end;
            size_t lastPos = 0;

            for (; it != end; ++it) {
                result += line.substr(lastPos, it->position() - lastPos);           // unmatched part
                result += COLOR_YELLOW + it->str() + COLOR_RESET;                  // matched part
                lastPos = it->position() + it->length();
            }

            result += line.substr(lastPos);  // remaining unmatched

        } catch (const std::regex_error&) {
            return line + " [regex error]";
        }
    }

    return result;
}

/**
 * @brief Searches the specified file for the query.
 *
 * Reads the file line by line, searches for matches according to the provided options,
 * and prints matched lines to the console (highlighted if requested).
 *
 * @param filePath       The path to the file to search.
 * @param query          The query string or regex pattern.
 * @param caseSensitive  Whether the search should be case-sensitive.
 * @param highlight      Whether to highlight matches in the output.
 * @param useRegex       Whether to interpret the query as a regex.
 */
void TextFileSearcher::search(const std::filesystem::path &filePath, const std::string &query,
                              bool caseSensitive, bool highlight, bool useRegex, const std::string &threadIdStr)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }

    std::regex pattern;
    std::string loweredQuery;
    bool validRegex = true;

    // Preprocess pattern or loweredQuery outside loop
    if (useRegex) {
        std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
        if (!caseSensitive)
            flags |= std::regex::icase;
        try {
            pattern = std::regex(query, flags);
        }
        catch (...) {
            validRegex = false;
        }
    }
    else if (!caseSensitive) {
        loweredQuery = query;
        std::transform(loweredQuery.begin(), loweredQuery.end(), loweredQuery.begin(), ::tolower);
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        bool matched = false;

        if (useRegex && validRegex) {
            matched = std::regex_search(line, pattern);
        }
        else if (!useRegex && !caseSensitive) {
            std::string loweredLine = line;
            std::transform(loweredLine.begin(), loweredLine.end(), loweredLine.begin(), ::tolower);
            matched = loweredLine.find(loweredQuery) != std::string::npos;
        }
        else if (!useRegex) {
            matched = line.find(query) != std::string::npos;
        }

        if (matched) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << filePath.string() << ":" << lineNumber << ": [Thread " << threadIdStr << "] ";
            if (highlight)
                std::cout << highlightMatches(line, query, caseSensitive, useRegex) << std::endl;
            else
                std::cout << line << std::endl;
        }
    }
}

/**
 * @brief Constructs the SearchManager.
 *
 * @param searcher       A unique pointer to a FileSearcher implementation.
 * @param query          The search term or pattern.
 * @param caseSensitive  Whether the search should be case-sensitive.
 * @param highlight      Whether to highlight the matches.
 * @param useRegex       Whether the query is a regex pattern.
 */
SearchManager::SearchManager(std::unique_ptr<FileSearcher> searcher,
                             const std::string& query,
                             bool caseSensitive,
                             bool highlight,
                             bool useRegex)
    : searcher_(std::move(searcher)), query_(query),
      caseSensitive_(caseSensitive), highlight_(highlight), useRegex_(useRegex) {}

/**
 * @brief Performs recursive search across all files in the directory.
 *
 * Scans the directory recursively for regular files, then uses multiple threads to search
 * through those files in parallel. Each thread receives an even slice of files.
 *
 * @param dirPath The path to the root directory for searching.
 */
void SearchManager::searchInDirectory(const std::filesystem::path& dirPath) {
    if (!std::filesystem::is_directory(dirPath)) {
        std::cerr << "Error: Invalid directory - " << dirPath << std::endl;
        return;
    }

    // Recursively gather all regular files in the directory 
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            files.emplace_back(entry.path());
        }
    }

    size_t threadCount = std::min(numThreads_, files.size());
    size_t filesPerThread = files.size() / threadCount;
    size_t remainingFiles = files.size() % threadCount;

    std::vector<std::thread> threads;
    auto beginIt = files.begin();

    for (size_t i = 0; i < threadCount; ++i) {
        // Calculate start and end iterators for each thread
        auto endIt = beginIt + filesPerThread + (i < remainingFiles ? 1 : 0);

        threads.emplace_back([this, beginItCopy = beginIt, endItCopy = endIt]() {
            std::thread::id thisId = std::this_thread::get_id();
            int threadNumber;

            {
                std::lock_guard<std::mutex> lock(threadIdMutex_);
                auto [it, inserted] = threadIdMap_.try_emplace(thisId, nextThreadId_++);
                threadNumber = it->second;
            }

            // {
            //     std::lock_guard<std::mutex> lock(TextFileSearcher::coutMutex);
            //     std::cout << "Thread " << threadNumber << " assigned files: ";
            //     for (auto it = beginItCopy; it != endItCopy; ++it)
            //         std::cout << "  " << it->string() << "\n";
            // }

            {
                std::lock_guard<std::mutex> lock(TextFileSearcher::coutMutex);
                std::cout << "Thread " << threadNumber << " assigned " 
                          << std::distance(beginItCopy, endItCopy) << " file(s)\n";
            }

            for (auto it = beginItCopy; it != endItCopy; ++it) {
                searcher_->search(*it, query_, caseSensitive_, highlight_, useRegex_, std::to_string(threadNumber));
            }
        });

        beginIt = endIt; // Advance to next chunk
    }

    // Wait for all threads to complete execution
    for (auto& thread : threads) {
        thread.join();
    }
}

