cmake_minimum_required(VERSION 3.21)
project(Blocking_Concurrent_Queue)

set(CMAKE_CXX_STANDARD 23)

add_executable(Blocking_Concurrent_Queue main.cpp Non_Block.h two_lock.h)
