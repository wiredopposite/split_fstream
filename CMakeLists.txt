cmake_minimum_required(VERSION 3.13)

add_library(split_fstream INTERFACE)

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(split_fstream INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/split_ofstream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/split_ifstream.cpp
)

target_include_directories(split_fstream INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src
)