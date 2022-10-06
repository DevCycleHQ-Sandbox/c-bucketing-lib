cmake_minimum_required(VERSION 3.23)
project(c_bucketing_lib C)
set(CMAKE_C_STANDARD 11)
add_library(c_bucketing_lib bucketing.c)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/wasmtime/crates/c-api
        ${CMAKE_CURRENT_BINARY_DIR}/wasmtime)

target_include_directories(c_bucketing_lib PUBLIC wasmtime)
target_link_libraries(c_bucketing_lib PUBLIC wasmtime)
