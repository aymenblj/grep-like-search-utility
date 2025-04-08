# Collect all the source files except for main.cpp
file(GLOB SOURCES "*.cpp")

# Exclude main.cpp from the list of sources
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/main.cpp")

# Create a library from the remaining sources
add_library(FileSearcherLib ${SOURCES})

# Include the header files
target_include_directories(FileSearcherLib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Specify the source files for the executable
file(GLOB EXE_SOURCES "*.cpp")

# Create the executable for testing or running the application
add_executable(FileSearcher ${EXE_SOURCES})

# Link the library to the executable
target_link_libraries(FileSearcher PRIVATE FileSearcherLib)
