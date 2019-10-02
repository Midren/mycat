cmake_minimum_required(VERSION 3.14)
project(mycat)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost)

add_executable(mycat main.cpp)
target_link_libraries(mycat boost_program_options)