cmake_minimum_required(VERSION 3.10)
project(CM)

include_directories(./ ../ ../../common/ ../../queue/ ../../sketch/)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -faligned-new -pthread -mavx2")
#set(CMAKE_CXX_FLAGS "-O0 -Wall -g -ggdb -pthread")

add_executable(CM main.cpp)
target_link_libraries(CM)