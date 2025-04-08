#ifndef GREPLIKEUTILITY_HPP
#define GREPLIKEUTILITY_HPP

#include <string>
#include <filesystem>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>

/**
 * @brief Abstract interface for performing query search.
 *
 * A polymorphic interface for searching files.
 * Implementations must define how the search is conducted.
 * 
 * Notes: 
 * std::unique_ptr will automatically delete SearchManager copy constructor
 * Custom Destructors: The classes FileSearcher, TextFileSearcher, and SearchManager do not contain any raw pointers 
 * or resources that require manual memory management. Therefore, the default destructor will be sufficient.
 */
class FileSearcher {
public:
    /**
     * @brief Searches a specified file for the query.
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
    virtual void search(const std::filesystem::path& filePath, const std::string& query,
                        bool caseSensitive, bool highlight, bool useRegex) = 0;

    virtual ~FileSearcher() = default;
};

/**
 * @brief Concrete implementation of FileSearcher for text files.
 *
 * Searches through plain text files, supporting literal and regex patterns,
 * with optional case sensitivity, and highlighting of matches.
 */
class TextFileSearcher : public FileSearcher {
public:
    void search(const std::filesystem::path& filePath, const std::string& query,
                bool caseSensitive, bool highlight, bool useRegex) override;

private:
    ///< Mutex to synchronize multi-threaded console output.
    static std::mutex coutMutex;
};

/**
 * @brief Manage file search operations across directories using threads.
 *
 * Coordinates multi-threaded file searching with optional regex and highlighting.
 */
class SearchManager {
public:
    /**
     * @brief Constructor.
     *
     * @param searcher       A unique pointer to a FileSearcher implementation.
     * @param query          The search term or pattern.
     * @param caseSensitive  Whether the search should be case-sensitive.
     * @param highlight      Whether to highlight the matches.
     * @param useRegex       Whether the query is a regex pattern.
     */
    SearchManager(std::unique_ptr<FileSearcher> searcher, const std::string& query,
                  bool caseSensitive = true, bool highlight = false, bool useRegex = false);

    /**
     * @brief Performs recursive search across all files in the directory.
     *
     * Scans the directory recursively for regular files, then uses multiple threads to search
     * through those files in parallel. Each thread receives an even slice of files.
     *
     * @param dirPath The path to the root directory for searching.
     */
    void searchInDirectory(const std::filesystem::path& dirPath);

private:
    std::unique_ptr<FileSearcher> searcher_;  ///< The searcher implementation to use. 
    std::string query_;                       ///< The search term or pattern.
    bool caseSensitive_;                      ///< Whether the search is case-sensitive.
    bool highlight_;                          ///< Whether to highlight the matches.
    bool useRegex_;                           ///< Whether the query is a regex.
    size_t numThreads_ = std::thread::hardware_concurrency(); ///< Number of threads to use.
};

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
std::string highlightMatches(const std::string& line, const std::string& query,
                             bool caseSensitive, bool useRegex);

#endif  // GREPLIKEUTILITY_HPP
