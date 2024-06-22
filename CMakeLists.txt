# cmake_minimum_required(VERSION 3.13)

# add_library(split_fstream INTERFACE)

# include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

# target_sources(split_fstream INTERFACE
#     ${CMAKE_CURRENT_LIST_DIR}/src/split_ofstream.cpp
#     ${CMAKE_CURRENT_LIST_DIR}/src/split_ifstream.cpp
# )

# target_include_directories(split_fstream INTERFACE
#     ${CMAKE_CURRENT_LIST_DIR}/src
# )

cmake_minimum_required(VERSION 3.11)

project(split_fstream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(split_fstream STATIC src/split_ifstream.cpp src/split_ofstream.cpp)

target_include_directories(split_fstream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
