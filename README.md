# Chess Qt

Simple chess game implemented in Qt.

## Build

Requires Qt 6 and CMake.
On Ubuntu based systems Qt 6 can be installed with:

```bash
sudo apt-get install qt6-base-dev
```

```bash
cmake -S . -B build
cmake --build build
```

Stockfish is included as a subdirectory. Build it with:

```bash
make -C stockfish/src -j profile-build
```

The executable will be placed under `build/src/` so run `./src/chessqt`
from inside the `build` directory.
