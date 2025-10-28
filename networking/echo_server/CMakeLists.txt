cmake_minimum_required(VERSION 3.31)
project(echo_server)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(echo_server main.cpp)

target_include_directories(echo_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(echo_server PRIVATE ${Boost_LIBRARIES})

target_compile_options(echo_server PRIVATE -Wextra -Werror)