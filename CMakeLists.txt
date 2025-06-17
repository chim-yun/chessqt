cmake_minimum_required(VERSION 3.16)
project(chessqt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

add_custom_target(build_stockfish ALL
    COMMAND ${CMAKE_MAKE_PROGRAM} build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/stockfish/engine/src
    COMMENT "Building Stockfish engine"
)

add_custom_command(TARGET build_stockfish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/stockfish/engine/src/stockfish
            ${CMAKE_CURRENT_SOURCE_DIR}/stockfish/engine/stockfish
)

add_subdirectory(src)

add_dependencies(chessqt build_stockfish)
