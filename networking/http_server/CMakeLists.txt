cmake_minimum_required(VERSION 3.31)
project(http_server)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(http_server main.cpp)

target_include_directories(http_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(http_server PRIVATE ${Boost_LIBRARIES})

target_compile_options(http_server PRIVATE -Wextra -Werror)