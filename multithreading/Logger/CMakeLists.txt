cmake_minimum_required(VERSION 3.31)
project(Logger)

set(CMAKE_CXX_STANDARD 20)

add_executable(Logger main.cpp)

target_compile_options(Logger PRIVATE -Wextra -Werror)