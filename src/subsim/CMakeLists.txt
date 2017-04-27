include(../../init.cmake)

project(subsim)
include_directories(. ../db ../utils)
aux_source_directory(. SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} db utils)

