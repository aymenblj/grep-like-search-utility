#include <gtest/gtest.h>
#include "grepLikeUtility.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

// Helper function to create test files in examples/
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file("examples/" + filename);
    file << content;
    file.close();
}

class GrepUtilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::create_directory("examples");
        createTestFile("test1.txt", "Hello World\nhello earth\nHELLO Galaxy");
        createTestFile("test2.txt", "Testing colors\nAnother line\ncolors again");
        createTestFile("test3.txt", "No matches here.");
    }

    void TearDown() override {
        std::filesystem::remove_all("examples");
    }
};

TEST_F(GrepUtilityTest, CaseSensitiveSearchShouldMatchExact) {
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test1.txt", "Hello", true, false, false);  // Adding useRegex flag
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("examples/test1.txt:1: Hello World") != std::string::npos);
    EXPECT_FALSE(output.find("examples/test1.txt:2: hello earth") != std::string::npos);
}

TEST_F(GrepUtilityTest, CaseInsensitiveSearchShouldMatchAllVariants) {
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test1.txt", "hello", false, false, false);  // Adding useRegex flag
    std::string output = testing::internal::GetCapturedStdout();        
    EXPECT_TRUE(output.find("examples/test1.txt:1: Hello World") != std::string::npos);
    EXPECT_TRUE(output.find("examples/test1.txt:2: hello earth") != std::string::npos);
    EXPECT_TRUE(output.find("examples/test1.txt:3: HELLO Galaxy") != std::string::npos);
}

TEST_F(GrepUtilityTest, HighlightedOutputShouldContainColorCodes) {
    std::string line = "Color test: colors and more COLORS";
    std::string highlighted = highlightMatches(line, "colors", false, false);  // Use literal match
    EXPECT_NE(highlighted.find("\033[33m"), std::string::npos);
    EXPECT_NE(highlighted.find("\033[0m"), std::string::npos);
}

TEST_F(GrepUtilityTest, RegexSearchShouldMatchPattern) {
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test2.txt", "colo.*", false, false, true);  // Adding useRegex flag
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("examples/test2.txt:1: Testing colors") != std::string::npos);
    EXPECT_TRUE(output.find("examples/test2.txt:3: colors again") != std::string::npos);
}

TEST_F(GrepUtilityTest, RegexSearchWithCaseInsensitiveFlag) {
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test1.txt", "HELLO", false, false, true);  // Adding useRegex flag
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("examples/test1.txt:3: HELLO Galaxy") != std::string::npos);
}

TEST_F(GrepUtilityTest, NoMatchShouldProduceNoOutput) {
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test3.txt", "unmatched", true, false, false);  // Adding useRegex flag
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty());
}

TEST_F(GrepUtilityTest, InvalidDirectoryShouldPrintError) {
    std::unique_ptr<FileSearcher> searcher = std::make_unique<TextFileSearcher>();
    SearchManager manager(std::move(searcher), "anything", true, false, false);  // Adding useRegex flag
    testing::internal::CaptureStderr();
    manager.searchInDirectory("invalid_directory");
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_NE(err.find("Error: Invalid directory"), std::string::npos);
}

TEST_F(GrepUtilityTest, RecursiveSearchFindsInAllFiles) {
    std::unique_ptr<FileSearcher> searcher = std::make_unique<TextFileSearcher>();
    SearchManager manager(std::move(searcher), "colors", false, false, false);  // Adding useRegex flag
    testing::internal::CaptureStdout(); 
    // Make sure the manager searches without highlighting colors
    manager.searchInDirectory("examples");  // Set highlight to false here
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("test2.txt") != std::string::npos);
    EXPECT_TRUE(output.find("colors again") != std::string::npos);
}

TEST_F(GrepUtilityTest, ThreadedSearchExecutesCorrectly) {
    std::unique_ptr<FileSearcher> searcher = std::make_unique<TextFileSearcher>();
    SearchManager manager(std::move(searcher), "hello", false, false, false);  // Adding useRegex flag
    testing::internal::CaptureStdout();
    manager.searchInDirectory("examples");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("test1.txt") != std::string::npos);
}

TEST_F(GrepUtilityTest, RegexSpecialCharactersShouldBeHandled) {
    createTestFile("test_regex.txt", "Question? Dot. Star*");
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/test_regex.txt", "Question\\?", false, false, true);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Question?") != std::string::npos);
}

TEST_F(GrepUtilityTest, EmptyFileShouldProduceNoOutput) {
    createTestFile("empty.txt", "");
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/empty.txt", "hello", false, false, false);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty());
}

TEST_F(GrepUtilityTest, FileWithOnlyNewlinesShouldNotMatch) {
    createTestFile("only_newlines.txt", "\n\n\n");
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/only_newlines.txt", "hello", false, false, false);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty());
}

TEST_F(GrepUtilityTest, HighlightWithRegexShouldColorAllMatches) {
    std::string line = "highlight this highlight that";
    std::string highlighted = highlightMatches(line, "highlight", false, true); // regex
    EXPECT_NE(highlighted.find("\033[33mhighlight\033[0m"), std::string::npos);
}

TEST_F(GrepUtilityTest, MultipleMatchesInOneLine) {
    createTestFile("multi.txt", "test test test");
    TextFileSearcher searcher;
    testing::internal::CaptureStdout();
    searcher.search("examples/multi.txt", "test", false, false, false);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("test test test") != std::string::npos);
}
