cmake_minimum_required (VERSION 3.0)
project (TimeAttackCpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(external)
include_directories(external/spdlog/include)


set(SOURCES 
   src/main.cpp
   src/worker.h
   src/common.h
)

add_definitions(-DSPDLOG_NO_TLS)

add_executable(TimeAttackCpp ${SOURCES})