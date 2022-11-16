cmake_minimum_required(VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Idx2 CXX)

# experimental (!)
add_compile_definitions(VISUS_IDX2=1)

add_subdirectory(Source)
