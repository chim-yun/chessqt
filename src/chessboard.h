#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <array>
#include <QString>

class ChessBoard
{
public:
    enum Color { White, Black };
    enum Piece { Empty, WP, WR, WN, WB, WQ, WK, BP, BR, BN, BB, BQ, BK };

    ChessBoard();
    void reset();
    bool move(const QString &from, const QString &to);
    Piece pieceAt(int row, int col) const { return m_board[row*8+col]; }
    Color currentColor() const { return m_turn; }

private:
    using Board = std::array<Piece, 64>;
    Board m_board;
    Color m_turn = White;
};

#endif // CHESSBOARD_H
