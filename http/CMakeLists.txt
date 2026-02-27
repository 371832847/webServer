#

cmake_minimum_required(VERSION 3.0.0)

project(test)

include_directories(/usr/include/mysql)

link_directories(/lib64/)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

add_executable(test test.cpp httpConn.cpp httpRequest.cpp ../sqlConn/sqlConn.cpp httpResponse.cpp ../buffer/Buffer.cpp ../epoller/epoller.cpp)

target_link_libraries(test mysqlclient)