cmake_minimum_required(VERSION 3.23)
project(dvcbucketing C)
set(CMAKE_C_STANDARD 11)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/wasmtime/crates/c-api
        ${CMAKE_CURRENT_BINARY_DIR}/wasmtime)

add_library(dvcbucketing bucketing.c)
target_include_directories(dvcbucketing PUBLIC wasmtime)
target_link_libraries(dvcbucketing PUBLIC wasmtime)

add_executable(dvctest test/testing.c)
target_include_directories(dvctest PUBLIC wasmtime)
target_link_libraries(dvctest PUBLIC dvcbucketing)
