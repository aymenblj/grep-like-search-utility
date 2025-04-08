# Collect all test files
file(GLOB TEST_SOURCES "*.cpp")

# Create an executable for the unit tests
add_executable(unitTests ${TEST_SOURCES})

# Link the test executable with GoogleTest and the necessary parts of FileSearcherLib
target_link_libraries(unitTests GTest::gtest GTest::gtest_main FileSearcherLib)

# Add a custom target to run the tests using Ninja
add_custom_target(
  runUnitTests
  COMMAND unitTests --gtest_color=yes
  DEPENDS unitTests
)

# Ensure the tests are built before running
add_dependencies(runUnitTests unitTests)
