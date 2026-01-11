cmake_minimum_required(VERSION 3.10)
project(MapLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MAPLOG_BUILD_EXAMPLES "Build example programs" ON)

add_library(maplog src/logger.cpp)

target_include_directories(maplog
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(MAPLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()