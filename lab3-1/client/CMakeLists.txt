cmake_minimum_required(VERSION 3.22)
project(lab3_1_client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-static")
link_libraries(ws2_32 wsock32)
add_executable(lab3_1_client main.cpp)
