#include "chessboard.h"

ChessBoard::ChessBoard()
{
    reset();
}

void ChessBoard::reset()
{
    m_board.fill(Empty);
    const Piece init[64] = {
        BR, BN, BB, BQ, BK, BB, BN, BR,
        BP, BP, BP, BP, BP, BP, BP, BP,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        WP, WP, WP, WP, WP, WP, WP, WP,
        WR, WN, WB, WQ, WK, WB, WN, WR
    };
    for (int i=0;i<64;++i)
        m_board[i]=init[i];
    m_turn = White;
    m_history.clear();
}

static void strToPos(const QString &s,int &r,int &c)
{
    c = s[0].toLatin1()-'a';
    r = 7 - (s[1].digitValue()-1);
}

bool ChessBoard::move(const QString &from, const QString &to)
{
    QVector<QPoint> moves = legalMoves(from);
    int tr,tc; strToPos(to,tr,tc);
    bool legal=false;
    for (const QPoint &p : moves) {
        if (p.x()==tr && p.y()==tc) { legal=true; break; }
    }
    if (!legal)
        return false;
    int fr,fc; strToPos(from,fr,fc);
    int fi = fr*8+fc;
    int ti = tr*8+tc;
    m_board[ti] = m_board[fi];
    m_board[fi] = Empty;
    m_turn = (m_turn==White)?Black:White;
    m_history.append(from+to);
    return true;
}

ChessBoard::Color ChessBoard::pieceColor(Piece p) const
{
    if (p>=WP && p<=WK) return White;
    if (p>=BP && p<=BK) return Black;
    return White;
}

QVector<QPoint> ChessBoard::legalMoves(const QString &from) const
{
    QVector<QPoint> res;
    int r,c; strToPos(from,r,c);
    Piece p = pieceAt(r,c);
    if (p==Empty) return res;
    Color col = pieceColor(p);
    auto add=[&](int rr,int cc){
        if (rr<0||rr>=8||cc<0||cc>=8) return false;
        Piece t = pieceAt(rr,cc);
        if (t==Empty) { res.append(QPoint(rr,cc)); return true; }
        if (pieceColor(t)!=col) { res.append(QPoint(rr,cc)); return false; }
        return false;
    };

    if (p==WP || p==BP) {
        int dir = (p==WP)?-1:1;
        int startRow = (p==WP)?6:1;
        if (pieceAt(r+dir,c)==Empty) add(r+dir,c);
        if (r==startRow && pieceAt(r+dir,c)==Empty && pieceAt(r+2*dir,c)==Empty) add(r+2*dir,c);
        if (c>0) {
            Piece t = pieceAt(r+dir,c-1);
            if (t!=Empty && pieceColor(t)!=col) res.append(QPoint(r+dir,c-1));
        }
        if (c<7) {
            Piece t = pieceAt(r+dir,c+1);
            if (t!=Empty && pieceColor(t)!=col) res.append(QPoint(r+dir,c+1));
        }
        return res;
    }

    if (p==WN || p==BN) {
        const int d[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
        for (auto &o:d) add(r+o[0],c+o[1]);
        return res;
    }
    if (p==WB || p==BB || p==WQ || p==BQ) {
        const int d[4][2]={{-1,-1},{-1,1},{1,-1},{1,1}};
        for (auto &o:d){int rr=r+o[0],cc=c+o[1];while(add(rr,cc)){rr+=o[0];cc+=o[1];}}
        if (p==WB || p==BB) return res; // bishops only
    }
    if (p==WR || p==BR || p==WQ || p==BQ) {
        const int d[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
        for (auto &o:d){int rr=r+o[0],cc=c+o[1];while(add(rr,cc)){rr+=o[0];cc+=o[1];}}
        if (p==WR || p==BR) return res; // rooks only
    }
    if (p==WK || p==BK) {
        const int d[8][2]={{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
        for (auto &o:d) add(r+o[0],c+o[1]);
    }
    return res;
}
