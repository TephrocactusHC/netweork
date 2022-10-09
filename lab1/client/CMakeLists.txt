cmake_minimum_required(VERSION 3.22)
project(client)

set(CMAKE_CXX_STANDARD 14)
link_libraries(ws2_32 wsock32)
add_executable(client main.cpp)
