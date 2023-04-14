cmake_minimum_required(VERSION 3.17)
project(your_project_name)

set(CMAKE_CXX_STANDARD 14)

add_executable(your_executable_name main.cpp main.cpp)

target_link_libraries(your_executable_name PRIVATE pthread)

# add the following lines
find_package(Threads REQUIRED)
target_link_libraries(your_executable_name PRIVATE Threads::Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")
