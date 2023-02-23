cmake_minimum_required(VERSION 2.8.12)
project(BinarySearchTreeBenchmark)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR}/include/)

add_executable(main src/main.cpp)
target_link_libraries(main ${CONAN_LIBS})