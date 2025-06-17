# Chess Qt

Simple chess game implemented in Qt.

## Build

Requires Qt 6 and CMake.

```bash
cmake -S . -B build
cmake --build build
```

On Windows, the build system will automatically search for `mingw32-make`
when compiling the bundled Stockfish engine and fall back to `make` if
`mingw32-make` is not available. This allows the project to build with the
default MinGW toolchain that ships with Qt.

The build will also compile the bundled **Stockfish** engine. The resulting
engine binary is copied to `stockfish/engine/stockfish` so that playing
against the AI works out of the box.

Run `./chessqt` inside the `build` directory to start the application.
