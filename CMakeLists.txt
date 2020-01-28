cmake_minimum_required(VERSION 3.14)
project(podofo_poc)

set(CMAKE_CXX_STANDARD 17)

add_executable(podofo_poc main.cpp)
target_link_libraries(podofo_poc podofo)
