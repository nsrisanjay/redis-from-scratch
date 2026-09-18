cmake_minimum_required(VERSION 3.16)

project(redis_from_scratch
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(redis-server
    src/main.cpp
)

target_compile_options(redis-server PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(tcp_tests
    tests/tcp_tests.cpp
)

target_compile_options(tcp_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)