cmake_minimum_required(VERSION 2.8.4)
project(os1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    osm.cpp
    osm.h)

add_executable(os1 ${SOURCE_FILES})