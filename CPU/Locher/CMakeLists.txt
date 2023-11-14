cmake_minimum_required(VERSION 3.10)
project(Locher)

include_directories(./ ../ ../../common/ ../../queue/ ../../sketch/)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -faligned-new -pthread -mavx")
#set(CMAKE_CXX_FLAGS "-O0 -Wall -g -ggdb -pthread")

add_executable(Locher main.cpp)
target_link_libraries(Locher)