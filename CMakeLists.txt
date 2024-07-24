cmake_minimum_required(VERSION 3.27)
project(PA2)

set(CMAKE_CXX_STANDARD 23)

add_executable(PA2 excel.cpp)
target_link_libraries(PA2 PRIVATE -L${CMAKE_SOURCE_DIR}/x86_64-linux-gnu -lexpression_parser)
