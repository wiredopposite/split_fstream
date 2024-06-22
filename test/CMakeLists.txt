cmake_minimum_required(VERSION 3.11)

project(fstream_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SPLIT_FSTREAM_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
add_subdirectory(${SPLIT_FSTREAM_ROOT} ${CMAKE_BINARY_DIR}/split_fstream)

add_executable(${PROJECT_NAME} main.cpp sha256.cpp)

target_link_libraries(${PROJECT_NAME} split_fstream)
