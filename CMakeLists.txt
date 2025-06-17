cmake_minimum_required(VERSION 3.16)
project(chessqt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

add_subdirectory(src)
