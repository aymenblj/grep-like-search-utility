# ---------------------------------------------------------------------------------------
# Third-Parties Setup via FetchContent
#
# This configuration fetches and builds a custom directory inside the build tree 
# (`external/Third-Party`).
#
# - Uses FetchContent to ensure reproducible builds without system-wide dependencies.
# - Keeps third-party code out of the source tree.
# - Compatible with Ninja and multi-platform toolchains (Linux, Windows, macOS).
#
# The resulting GTest targets (`gtest`, `gtest_main`) can be linked with your tests.
# ---------------------------------------------------------------------------------------
include(FetchContent)

set(EXTERNAL_LOCATION ${CMAKE_BINARY_DIR}/external)
set(GOOGLETEST_LOCATION ${EXTERNAL_LOCATION}/googletest)
set(GTEST_ALL_BINARY_DIR ${GOOGLETEST_LOCATION}/googletest-build)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  SOURCE_DIR     ${GOOGLETEST_LOCATION}/googletest-src
  BINARY_DIR     ${GTEST_ALL_BINARY_DIR}
  SUBBUILD_DIR   ${GOOGLETEST_LOCATION}/googletest-subbuild
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${GTEST_ALL_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
