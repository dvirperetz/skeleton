cmake_minimum_required(VERSION 3.12)
project(skeleton)

set(CMAKE_CXX_STANDARD_REQUIRED 11)

add_executable(skeleton Commands.cpp Commands.h signals.cpp signals.h smash.cpp)
set(CMAKE_CXX_FLAGS "-Wall")