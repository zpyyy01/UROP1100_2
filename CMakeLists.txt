cmake_minimum_required(VERSION 3.10)
project(ReplaceEqualOperator)

set(CMAKE_CXX_STANDARD 17)

# Find Clang package
find_package(Clang REQUIRED CONFIG)

# Include Clang headers
include_directories(${CLANG_INCLUDE_DIRS})

# Add libraries
add_library(MyLibrary STATIC src/ASTvisitor.cpp)
add_library(MyLibrary2 STATIC src/ReplaceEqualOperatorVisitor.cpp)

# Include directories for headers
include_directories(include src)

# Add executable
add_executable(a src/main.cpp)

# Link the executable against the libraries and Clang libraries
target_link_libraries(a MyLibrary MyLibrary2 clangTooling clangBasic clangAST clangEdit)