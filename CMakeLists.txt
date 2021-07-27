cmake_minimum_required(VERSION 3.16)
project(Media_sorter)

set(CMAKE_CXX_STANDARD 20)

set(HEADERS lib/TinyEXIF.h lib/tinyxml2.h)
set(SOURCES main.cpp lib/TinyEXIF.cpp lib/tinyxml2.cpp)

add_executable(Media_sorter main.cpp ${HEADERS} ${SOURCES})