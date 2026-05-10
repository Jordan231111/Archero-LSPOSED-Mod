cmake_minimum_required(VERSION 3.10.2)
project("archero_mod")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(archero_mod SHARED mod.cpp And64InlineHook.cpp)

target_compile_definitions(archero_mod PRIVATE MOD_ENABLE_DEBUG_LOGS=1 NDEBUG)
target_compile_options(
    archero_mod PRIVATE
    -O3
    -g0
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -s")

target_link_libraries(archero_mod log dl)
