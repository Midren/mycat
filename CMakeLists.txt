cmake_minimum_required(VERSION 3.13)
project(mycat)

#set(BOOST_ROOT /usr/local/Cellar/boost)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER i386-jos-elf-gcc)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(mycat main.cpp)
target_link_libraries(mycat ${Boost_LIBRARIES})
message(${CMAKE_INSTALL_PREFIX})

install (TARGETS mycat EXPORT mycat RUNTIME DESTINATION /Users/admin/Desktop/POK/myshell/bin)
export(EXPORT mycat FILE "${CMAKE_CURRENT_BINARY_DIR}/../mycatTargets.cmake")