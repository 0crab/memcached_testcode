cmake_minimum_required(VERSION 3.15)

project(batch_testcode)
set(CMAKE_CXX_STANDARD 11)
link_libraries(pthread)

add_executable(memcachedbinary_client memcachedbinary_client.cc)

add_executable(set_test multitest/set_test.cpp)