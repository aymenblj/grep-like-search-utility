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
        // Handle plain substring matching
        std::string queryToFind = query;
        std::string lineContent = line;

        // If not case-sensitive, convert both the line and query to lowercase for comparison
        if (!caseSensitive) {
            std::transform(queryToFind.begin(), queryToFind.end(), queryToFind.begin(), ::tolower);
            std::string loweredLine = lineContent;
            std::transform(loweredLine.begin(), loweredLine.end(), loweredLine.begin(), ::tolower);

            size_t position = 0;
            while (position < loweredLine.size()) {
                size_t found = loweredLine.find(queryToFind, position);
                if (found == std::string::npos) {
                    result += lineContent.substr(position);  // Append remaining text
                    break;
                }
                // Append unmatched part and then highlight the matched substring
                result += lineContent.substr(position, found - position);
                result += COLOR_YELLOW + lineContent.substr(found, query.length()) + COLOR_RESET;
                position = found + query.length();
            }
        } else {
            // Case-sensitive substring search
            size_t position = 0;
            while (position < lineContent.size()) {
                size_t found = lineContent.find(query, position);
                if (found == std::string::npos) {
                    result += lineContent.substr(position);
                    break;
                }
                result += lineContent.substr(position, found - position);
                result += COLOR_YELLOW + lineContent.substr(found, query.length()) + COLOR_RESET;
                position = found + query.length();
            }
        }
    } else {
        // Use regex matching
        std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
        if (!caseSensitive) flags |= std::regex::icase;

        try {
            std::regex pattern(query, flags);
            std::sregex_iterator begin(line.begin(), line.end(), pattern), end;
            size_t lastPos = 0;

            // Iterate through all matches and color them
            for (auto it = begin; it != end; ++it) {
                auto match = *it;
                result += line.substr(lastPos, match.position() - lastPos);
                result += COLOR_YELLOW + match.str() + COLOR_RESET;
                lastPos = match.position() + match.length();
            }
            // Append any remaining part of the line
            result += line.substr(lastPos);  
        } catch (std::regex_error&) {
            // Handle invalid regex pattern gracefully
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
void TextFileSearcher::search(const std::filesystem::path& filePath, const std::string& query,
                              bool caseSensitive, bool highlight, bool useRegex) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;

    // Process file line by line
    while (std::getline(file, line)) {
        lineNumber++;
        bool matched = false;
        std::string currentLine = line;

        if (!useRegex && !caseSensitive) {
            // Transform to Lowercase both line and query for comparison
            std::string loweredLine = currentLine;
            std::string loweredQuery = query;
            std::transform(loweredLine.begin(), loweredLine.end(), loweredLine.begin(), ::tolower);
            std::transform(loweredQuery.begin(), loweredQuery.end(), loweredQuery.begin(), ::tolower);
            matched = loweredLine.find(loweredQuery) != std::string::npos;
        } else if (!useRegex) {
            // Plain case-sensitive substring search
            matched = currentLine.find(query) != std::string::npos;
        } else {
            // Regex search with error handling
            try {
                std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
                if (!caseSensitive) flags |= std::regex::icase;
                std::regex pattern(query, flags);
                matched = std::regex_search(currentLine, pattern);
            } catch (...) {
                matched = false;
            }
        }

        if (matched) {
            // Ensure the printing is thread-safe
            std::lock_guard<std::mutex> lock(coutMutex);  
            std::string outputLine = highlight ? highlightMatches(line, query, caseSensitive, useRegex) : line;
            std::cout << filePath.string() << ":" << lineNumber << ": " << outputLine << std::endl;
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
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Error: Invalid directory - " << dirPath << std::endl;
        return;
    }

    // Recursively gather all regular files in the directory 
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    // Divide files over threads
    size_t filesPerThread = files.size() / numThreads_;
    std::vector<std::thread> threads;

    for (size_t  i = 0; i < numThreads_; ++i) {
        // Calculate range of files for each thread
        auto start = files.begin() + i * filesPerThread;
        auto end = (i == numThreads_ - 1) ? files.end() : start + filesPerThread;

        // Create a new thread to process a subset of files from 'start' to 'end'
        threads.emplace_back([this, start, end]() {
            // Iterate through the files in the specified range (from 'start' to 'end')
            for (auto it = start; it != end; ++it) {
                // Perform the search operation for each file in the range
                searcher_->search(*it, query_, caseSensitive_, highlight_, useRegex_);
            }
        });
    }

    // Wait for all threads to complete execution
    for (auto& thread : threads) {
        thread.join();
    }
}
