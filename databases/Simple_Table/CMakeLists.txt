cmake_minimum_required(VERSION 3.31)
project(Simple_Table)

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/include)

add_executable(Simple_Table main.cpp)

target_link_libraries(Simple_Table /opt/homebrew/lib/libpqxx.dylib)