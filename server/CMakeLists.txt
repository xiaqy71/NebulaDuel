cmake_minimum_required(VERSION 3.20)

project(NebulaDuelServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog REQUIRED)

add_executable(nebula_server
    src/main.cpp
    src/net/EpollPoller.cpp
    src/net/EventLoop.cpp
)

target_include_directories(nebula_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(nebula_server
    PRIVATE
        spdlog::spdlog
)

target_compile_options(nebula_server
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
