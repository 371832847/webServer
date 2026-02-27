#Cmake


cmake_minimum_required(VERSION 3.0.0)

project(server)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/include/mysql)

link_directories(/lib64/)

set(DIR /root/projects/webchat)
set(EPOLL ${DIR}/epoller/epoller.cpp)
set(SQLCONN ${DIR}/sqlConn/sqlConn.cpp)
set(THREADPOOL ${DIR}/threadPool/threadPool.h)
set(BUFFER ${DIR}/buffer/Buffer.cpp)
set(TIMER ${DIR}/heapTimer/heapTimer.cpp)

file(GLOB LOG_ ${DIR}/log/log.cpp ${DIR}/log/blockqueue.h)
file(GLOB HTTP ${DIR}/http/http*.cpp)
file(GLOB SERVER ${DIR}/server/*.cpp)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")


set(MAIN_SRC ${EPOLL} ${SQLCONN} ${THREADPOOL} ${BUFFER} ${HTTP} ${SERVER} ${TIMER} ${LOG_})

message(STATUS "src dir: ${MAIN_SRC}")

add_executable(test ${MAIN_SRC})

target_link_libraries(test mysqlclient pthread)