cmake_minimum_required(VERSION 3.10)
project(MyProject VERSION 1.0 LANGUAGES CXX)

find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})

set(SOURCES
    test.cpp  # 确保文件扩展名为 .cpp
)

add_executable(my_executable ${SOURCES})
target_link_libraries(my_executable clangTooling clangBasic clangAST clangEdit)