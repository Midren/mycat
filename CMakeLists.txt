cmake_minimum_required(VERSION 3.13)
project(mycat)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(mycat main.cpp)
target_link_libraries(mycat ${Boost_LIBRARIES})
