cmake_minimum_required(VERSION 3.15)
project(http-server VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(${PROJECT_SOURCE_DIR}/include)


set(SOURCES
    src/main.cpp
    src/http_server.cpp
    src/thread_pool.cpp
)

add_executable(http-server ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(http-server PRIVATE Threads::Threads)

install(TARGETS http-server DESTINATION bin)
