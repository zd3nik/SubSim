cmake_minimum_required(VERSION 3.1)
include(../../init.cmake)

project(db)
include_directories(. ..)
file(GLOB HDR_LIST *.h)
file(GLOB SRC_LIST *.cpp)
add_library(${PROJECT_NAME} STATIC ${HDR_LIST} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} utils)
