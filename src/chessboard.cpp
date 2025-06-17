#include "chessboard.h"

ChessBoard::ChessBoard()
{
    reset();
}

void ChessBoard::reset()
{
    m_board.fill(Empty);
    const Piece init[64] = {
        WR, WN, WB, WQ, WK, WB, WN, WR,
        WP, WP, WP, WP, WP, WP, WP, WP,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        BP, BP, BP, BP, BP, BP, BP, BP,
        BR, BN, BB, BQ, BK, BB, BN, BR
    };
    for (int i=0;i<64;++i)
        m_board[i]=init[i];
    m_turn = White;
}

bool ChessBoard::move(const QString &from, const QString &to)
{
    // Simplified move: no legality checks
    int fr = (7 - (from[1].digitValue()-1));
    int fc = from[0].toLatin1()-'a';
    int tr = (7 - (to[1].digitValue()-1));
    int tc = to[0].toLatin1()-'a';
    int fi = fr*8+fc;
    int ti = tr*8+tc;
    if (fi<0||fi>=64||ti<0||ti>=64)
        return false;
    m_board[ti] = m_board[fi];
    m_board[fi] = Empty;
    m_turn = (m_turn==White)?Black:White;
    return true;
}
