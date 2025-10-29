cmake_minimum_required(VERSION 3.15)
project(http-server VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/http_server.cpp
    src/thread_pool.cpp
)

# Create executable
add_executable(http-server ${SOURCES})

# Link pthread library (required for threading)
find_package(Threads REQUIRED)
target_link_libraries(http-server PRIVATE Threads::Threads)

# Installation rules
install(TARGETS http-server DESTINATION bin)
