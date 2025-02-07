cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TextEditor VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find ncurses library
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Source files
set(SOURCES 
    editor.cpp
    main.cpp
)

# Create the executable
add_executable(editor ${SOURCES})

# Link ncurses library
target_link_libraries(editor ${CURSES_LIBRARIES})

# Enable compiler warnings (optional but recommended)
target_compile_options(editor PRIVATE -Wall -Wextra -Wpedantic)

# Installation rule (optional)
install(TARGETS editor DESTINATION bin)
