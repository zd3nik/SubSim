cmake_minimum_required(VERSION 3.1)
include(../../init.cmake)

project(subsim)
include_directories(. ..)
file(GLOB HDR_LIST *.h commands/*.h)
file(GLOB SRC_LIST *.cpp)
add_library(${PROJECT_NAME} STATIC ${HDR_LIST} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} db utils)
