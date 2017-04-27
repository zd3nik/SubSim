include(../init.cmake)

add_subdirectory(utils)
add_subdirectory(db)
add_subdirectory(subsim)

project(subsim-server)
include_directories(subsim db utils)
add_executable(${PROJECT_NAME} "ServerMain.cpp")
target_link_libraries(${PROJECT_NAME} subsim db utils)

