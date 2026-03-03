cmake_minimum_required(VERSION 3.10.2)
project("archero_mod")

add_library(archero_mod SHARED mod.cpp And64InlineHook.cpp)

target_link_libraries(archero_mod log)