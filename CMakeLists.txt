cmake_minimum_required (VERSION 3.0)
project (TimeAttackCpp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/CLI11)

include_directories(src)
include_directories(external)
include_directories(external/spdlog/include)


set(SOURCES 
   src/main.cpp
   src/worker.h
   src/result_functions.h
   src/common.h
)

add_definitions(-DSPDLOG_NO_TLS)

add_executable(TimeAttackCpp ${SOURCES})

target_link_libraries(TimeAttackCpp CLI11::CLI11)