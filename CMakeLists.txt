cmake_minimum_required(VERSION 3.0)

project(lib VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Header-only libraries
include_directories("include")

file(GLOB SRCS src/*)
add_executable(${PROJECT_NAME} ${SRCS})