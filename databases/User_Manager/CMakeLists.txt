cmake_minimum_required(VERSION 3.31)
project(User_Manager)

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/include)

include_directories(${CMAKE_SOURCE_DIR}/src/database)
include_directories(${CMAKE_SOURCE_DIR}/src/service)

add_executable(User_Manager src/main.cpp
        src/database/Database.cpp
        src/service/UserService.cpp)

target_link_libraries(User_Manager /opt/homebrew/lib/libpqxx.dylib)
