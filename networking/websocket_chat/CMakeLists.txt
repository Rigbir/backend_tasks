cmake_minimum_required(VERSION 3.31)
project(websocket_chat)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(websocket_chat main.cpp)

target_include_directories(websocket_chat PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(websocket_chat PRIVATE ${Boost_LIBRARIES})
target_link_libraries(websocket_chat PRIVATE OpenSSL::SSL OpenSSL::Crypto)

target_compile_options(websocket_chat PRIVATE -Wextra -Werror)