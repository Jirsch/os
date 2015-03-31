cmake_minimum_required(VERSION 3.1)
project(ex2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_library(ex2 ${SOURCE_FILES} Thread.cpp Thread.h uthreads.cpp uthreads.h)