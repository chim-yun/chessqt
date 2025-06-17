# ChessQt

A minimal Qt-based chess demo inspired by [djbauman/Qt-Chess](https://github.com/djbauman/Qt-Chess). The application showcases a simple login screen, a menu, and a basic chessboard UI. Piece movement is unrestricted and implemented in a functional style with stateless helpers.

## Build

Ensure Qt5 development files are installed (`qtbase5-dev`). Build using `qmake` and `make`:

```bash
qmake chessqt.pro
make
```

Run with `./chessqt`.
