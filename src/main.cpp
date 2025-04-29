#include "grepLikeUtility.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <locale>

int main(int argc, char* argv[]) {
    std::setlocale(LC_ALL, "en_US.UTF-8");

    if (argc < 3 || argc > 5) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <query> [-i] [--regex]" << std::endl;
        std::cout << "  <directory_path>: Path to the directory to search in\n"
                  << "  <query>: The search query or regex pattern\n"
                  << "  [--ignore-case]: Optional flag for case-insensitive search\n"
                  << "  [--regex]: Optional flag to treat the query as a regular expression\n"
                  << "Example: " << argv[0] << " ./test_dir \"search_term\" -i --regex\n";
        return 1;
    }

    std::filesystem::path directoryPath = argv[1];
    std::string query = argv[2];
    bool caseSensitive = true;
    bool useRegex = false;

    for (int i = 3; i < argc; ++i) {
        std::string flag = argv[i];
        if (flag == "--ignore-case") {
            caseSensitive = false;
        } else if (flag == "--regex") {
            useRegex = true;
        } else {
            std::cout << "Unknown option: " << flag << std::endl;
            return 1;
        }
    }

    std::cout << "Searching in directory: " << directoryPath << std::endl;
    std::cout << "Search query: \"" << query << "\"" << std::endl;
    std::cout << "Case-sensitive search: " << (caseSensitive ? "Yes" : "No") << std::endl;
    std::cout << "Regex search: " << (useRegex ? "Yes" : "No") << std::endl;

    std::unique_ptr<FileSearcher> textFileSearcher = std::make_unique<TextFileSearcher>();
    SearchManager searchManager(std::move(textFileSearcher), query, caseSensitive, true, useRegex);
    searchManager.searchInDirectory(directoryPath);

    return 0;
}
