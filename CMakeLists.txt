cmake_minimum_required(VERSION 3.10)
project(ReplaceEqualOperator)

find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})

# source: src/main.cpp
set(SOURCES src/main.cpp)

add_executable(a ${SOURCES})
target_link_libraries(a clangTooling clangBasic clangAST clangEdit)