cmake_minimum_required(VERSION 3.16)
project(chessqt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

# Build the bundled Stockfish engine. We cannot rely on ${CMAKE_MAKE_PROGRAM}
# here, because the main project might use Ninja while the engine ships with a
# traditional Makefile. Instead try to locate a suitable `make` implementation
# manually and use that to compile Stockfish.
if(WIN32)
    # On Windows the tool is typically called `mingw32-make` when using the
    # MinGW toolchain that ships with Qt. Fall back to `make` if not found.
    find_program(STOCKFISH_MAKE_EXE NAMES mingw32-make make)
else()
    # On Unix-like systems a plain `make` should be present.
    find_program(STOCKFISH_MAKE_EXE NAMES make REQUIRED)
endif()

add_custom_target(build_stockfish ALL
    COMMAND ${STOCKFISH_MAKE_EXE} build

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
