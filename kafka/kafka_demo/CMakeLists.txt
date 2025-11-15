cmake_minimum_required(VERSION 3.31)
project(kafka_demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(RDKAFKA rdkafka++)
    if(RDKAFKA_FOUND)
        list(APPEND RDKAFKA_LIBRARIES rdkafka)
    endif()
endif()

if(NOT RDKAFKA_FOUND)
    find_path(RDKAFKA_INCLUDE_DIRS
        NAMES librdkafka/rdkafkacpp.h
        PATHS 
            /usr/local/Cellar/librdkafka/2.12.1/include
            /usr/local/include 
            /usr/include 
            /opt/homebrew/include
    )
    
    find_library(RDKAFKA_CPP_LIBRARIES
        NAMES rdkafka++
        PATHS 
            /usr/local/Cellar/librdkafka/2.12.1/lib
            /usr/local/lib 
            /usr/lib 
            /opt/homebrew/lib
    )
    
    find_library(RDKAFKA_BASE_LIBRARIES
        NAMES rdkafka
        PATHS 
            /usr/local/Cellar/librdkafka/2.12.1/lib
            /usr/local/lib 
            /usr/lib 
            /opt/homebrew/lib
    )
    
    if(NOT RDKAFKA_INCLUDE_DIRS OR NOT RDKAFKA_CPP_LIBRARIES OR NOT RDKAFKA_BASE_LIBRARIES)
        message(FATAL_ERROR "librdkafka not found. Install via: brew install librdkafka")
    endif()
    
    set(RDKAFKA_LIBRARIES ${RDKAFKA_CPP_LIBRARIES} ${RDKAFKA_BASE_LIBRARIES})
endif()

include_directories(${RDKAFKA_INCLUDE_DIRS})

add_executable(kafka_demo
        src/main.cpp
        src/Kafka_Producer.cpp
        src/Kafka_Consumer.cpp
)

if(RDKAFKA_LIBRARY_DIRS)
    link_directories(${RDKAFKA_LIBRARY_DIRS})
endif()

target_link_libraries(kafka_demo
    ${RDKAFKA_LIBRARIES}
)

if(RDKAFKA_CFLAGS_OTHER)
    target_compile_options(kafka_demo PRIVATE ${RDKAFKA_CFLAGS_OTHER})
endif()
