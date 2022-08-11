cmake_minimum_required(VERSION 3.10)

project(sql_with_cpp VERSION 1.0 DESCRIPTION "Using SQL with C++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(hello-sqlite src/sqlite3.c src/hello-sqlite.cpp)

target_include_directories( hello-sqlite
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)

