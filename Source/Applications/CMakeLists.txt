cmake_minimum_required(VERSION 3.8)
project(idx2Applications CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(idx2App idx2App.cpp ../../README.md ./convert.py)
target_link_libraries(idx2App idx2 Threads::Threads)

if (NOT VISUS_IDX2)
	add_executable(idx2Samples idx2Samples.cpp)
	target_link_libraries(idx2Samples idx2 Threads::Threads)
ENDIF()
