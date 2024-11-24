# cmake needs this line
cmake_minimum_required(VERSION 3.8)

project(examples)

include_directories(../include)

add_executable(client client.cpp)
target_link_libraries(client websocketclient)

