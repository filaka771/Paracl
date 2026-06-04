cmake_minimum_required(VERSION 3.10)
project(Paracl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_subdirectory(vendor)
add_subdirectory(lib)
add_subdirectory(unittests)
