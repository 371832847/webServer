###########


cmake_minimum_required(VERSION 3.0.0)


project(test)

include_directories(/usr/include/mysql)

link_directories(/lib64/)

add_executable(test test.cpp sqlConn.cpp)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

target_link_libraries(test mysqlclient)


