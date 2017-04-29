cmake_minimum_required(VERSION 3.1)
include(../init.cmake)

add_subdirectory(utils)
add_subdirectory(db)
add_subdirectory(subsim)

project(subsim-server)
include_directories(.)
add_executable(${PROJECT_NAME} "ServerMain.cpp")
target_link_libraries(${PROJECT_NAME} subsim db utils)
