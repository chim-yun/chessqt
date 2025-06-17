#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <array>
#include <QString>
#include <QVector>
#include <QPoint>

class ChessBoard
{
public:
    enum Color { White, Black };
    enum Piece { Empty, WP, WR, WN, WB, WQ, WK, BP, BR, BN, BB, BQ, BK };

    ChessBoard();
    void reset();
    bool move(const QString &from, const QString &to);
    QVector<QPoint> legalMoves(const QString &from) const;
    bool isInCheck(Color c) const;
    bool hasMoves(Color c) const;
    Piece pieceAt(int row, int col) const { return m_board[row*8+col]; }
    Color currentColor() const { return m_turn; }
    Color pieceColor(Piece p) const;
    QVector<QString> history() const { return m_history; }

private:
    using Board = std::array<Piece, 64>;
    Board m_board;
    Color m_turn = White;
    QVector<QString> m_history;
    bool m_whiteKingMoved = false;
    bool m_blackKingMoved = false;
    bool m_whiteLeftRookMoved = false;
    bool m_whiteRightRookMoved = false;
    bool m_blackLeftRookMoved = false;
    bool m_blackRightRookMoved = false;
    QPoint m_enPassant{-1,-1};

    bool isSquareAttacked(int r,int c,Color by) const;
};

#endif // CHESSBOARD_H
