cmake_minimum_required(VERSION 3.10)

project(sql_with_cpp VERSION 1.0 DESCRIPTION "Using SQL with C++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

#sources
set(sources "hello-sqlite" "db-query" "db-prepare" "db-prepare2" "ch02-solution" "practice")

#executables
foreach(File IN LISTS sources)
    add_executable(${File} src/sqlite3.c src/${File}.cpp)

    target_include_directories( ${File}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
    )    
endforeach()

#executables using BWSQL.cpp
set(other_sources "bwsql-test")
foreach(File IN LISTS other_sources)
    add_executable(${File} src/sqlite3.c src/BWSQL.cpp src/${File}.cpp)

    target_include_directories( ${File}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
    )    
endforeach()

