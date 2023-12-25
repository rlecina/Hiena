cmake_minimum_required(VERSION 3.22.1)

project(Hiena)

add_subdirectory(src)

target_include_directories(Hiena PUBLIC include)
target_include_directories(Hiena PRIVATE src)
