cmake_minimum_required(VERSION 3.16)
project(COM CXX)


file(GLOB_RECURSE cpp_sources include/*.cpp include/*.hpp)
add_executable(COM include/com.hpp examples/simple.cpp)
target_include_directories(COM PUBLIC include/)

option(USE_CLANG on)

set_property(TARGET COM PROPERTY CXX_STANDARD 20)
set_target_properties(COM PROPERTIES LINKER_LANGUAGE CXX)