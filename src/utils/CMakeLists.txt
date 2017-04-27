include(../../init.cmake)

project(utils)
include_directories(.)
aux_source_directory(. SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
