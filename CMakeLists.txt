cmake_minimum_required(VERSION 3.20)

project(
    prism
    VERSION 0.1.0
    DESCRIPTION "High-performance zero-copy CLI archive and file inspector"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(prism src/main.cpp)

target_include_directories(prism PRIVATE
    include
    src
)

target_compile_options(prism PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wconversion
)

option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers ENABLED (ASan + UBSan)")
    target_compile_options(prism PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(prism PRIVATE -fsanitize=address,undefined)
endif()