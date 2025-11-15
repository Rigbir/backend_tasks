cmake_minimum_required(VERSION 3.31)

set(VCPKG_ROOT_PATH "")
if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT_PATH "$ENV{VCPKG_ROOT}")
elseif(EXISTS "/Users/marat/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_ROOT_PATH "/Users/marat/vcpkg")
endif()

if(VCPKG_ROOT_PATH)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures" FORCE)
endif()

project(redis_demo)

set(CMAKE_CXX_STANDARD 20)

if(VCPKG_ROOT_PATH)
    set(hiredis_DIR "${VCPKG_ROOT_PATH}/installed/arm64-osx/share/hiredis" CACHE PATH "Path to hiredis config")
endif()

find_package(hiredis REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(redis_demo
        src/main.cpp
        src/http_server.cpp
        src/redis_client.cpp
)

target_include_directories(redis_demo PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(redis_demo PRIVATE Boost::system hiredis::hiredis)