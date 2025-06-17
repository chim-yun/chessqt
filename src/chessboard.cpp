#include "chessboard.h"
#include "utils.h"
#include <numeric>
#include <algorithm>
#include <array>

ChessBoard::ChessBoard()
{
    reset();
}

void ChessBoard::reset()
{
    static constexpr Board init{{
        BR, BN, BB, BQ, BK, BB, BN, BR,
        BP, BP, BP, BP, BP, BP, BP, BP,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        Empty, Empty, Empty, Empty, Empty, Empty, Empty, Empty,
        WP, WP, WP, WP, WP, WP, WP, WP,
        WR, WN, WB, WQ, WK, WB, WN, WR
    }};
    m_board = init;
    m_turn = White;
    m_history.clear();
    m_whiteKingMoved = false;
    m_blackKingMoved = false;
    m_whiteLeftRookMoved = false;
    m_whiteRightRookMoved = false;
    m_blackLeftRookMoved = false;
    m_blackRightRookMoved = false;
    m_enPassant = QPoint(-1,-1);
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
    bool legal = std::any_of(moves.cbegin(), moves.cend(), [&](const QPoint &p){
        return p.x()==tr && p.y()==tc;
    });
    if (!legal)
        return false;
    int fr,fc; strToPos(from,fr,fc);
    int fi = fr*8+fc;
    int ti = tr*8+tc;

    Piece moving = m_board[fi];

    // handle castling
    if (moving==WK && abs(tc-fc)==2) {
        // king side or queen side
        if (tc>fc) { // king side
            m_board[tr*8+5] = m_board[tr*8+7];
            m_board[tr*8+7] = Empty;
        } else {
            m_board[tr*8+3] = m_board[tr*8+0];
            m_board[tr*8+0] = Empty;
        }
    }
    if (moving==BK && abs(tc-fc)==2) {
        if (tc>fc) {
            m_board[tr*8+5] = m_board[tr*8+7];
            m_board[tr*8+7] = Empty;
        } else {
            m_board[tr*8+3] = m_board[tr*8+0];
            m_board[tr*8+0] = Empty;
        }
    }

    // handle en passant capture
    if ((moving==WP || moving==BP) && QPoint(tr,tc)==m_enPassant) {
        int capR = (moving==WP)?tr+1:tr-1;
        m_board[capR*8+tc] = Empty;
    }

    m_board[ti] = moving;
    m_board[fi] = Empty;

    // pawn promotion to queen
    if (moving==WP && tr==0) m_board[ti] = WQ;
    if (moving==BP && tr==7) m_board[ti] = BQ;

    // update castling rights
    if (moving==WK) m_whiteKingMoved=true;
    if (moving==BK) m_blackKingMoved=true;
    if (fi==7*8+0) m_whiteLeftRookMoved=true;
    if (fi==7*8+7) m_whiteRightRookMoved=true;
    if (fi==0*8+0) m_blackLeftRookMoved=true;
    if (fi==0*8+7) m_blackRightRookMoved=true;

    // set en passant target
    m_enPassant = QPoint(-1,-1);
    if ((moving==WP && fr==6 && tr==4) || (moving==BP && fr==1 && tr==3))
        m_enPassant = QPoint((moving==WP)?5:2,tc);

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
        if (pieceAt(r+dir,c)==Empty)
            add(r+dir,c);
        if (r==startRow && pieceAt(r+dir,c)==Empty && pieceAt(r+2*dir,c)==Empty)
            add(r+2*dir,c);
        if (c>0) {
            Piece t = pieceAt(r+dir,c-1);
            if (t!=Empty && pieceColor(t)!=col)
                res.append(QPoint(r+dir,c-1));
        }
        if (c<7) {
            Piece t = pieceAt(r+dir,c+1);
            if (t!=Empty && pieceColor(t)!=col)
                res.append(QPoint(r+dir,c+1));
        }
        if (m_enPassant.x()!=-1 && r==m_enPassant.x()+((p==WP)?1:-1) && abs(c-m_enPassant.y())==1)
            res.append(m_enPassant);
    }

    if (p==WN || p==BN) {
        static const std::array<std::array<int,2>,8> d{{
            {{-2,-1}},{{-2,1}},{{-1,-2}},{{-1,2}},{{1,-2}},{{1,2}},{{2,-1}},{{2,1}}
        }};
        std::for_each(d.begin(), d.end(), [&](const auto &o){ add(r+o[0], c+o[1]); });
    }
    if (p==WB || p==BB || p==WQ || p==BQ) {
        static const std::array<std::array<int,2>,4> d{{ {{-1,-1}},{{-1,1}},{{1,-1}},{{1,1}} }};
        std::for_each(d.begin(), d.end(), [&](const auto &o){
            for(int rr=r+o[0], cc=c+o[1]; add(rr,cc); rr+=o[0], cc+=o[1]){}
        });
        if (p==WB || p==BB) {}
    }
    if (p==WR || p==BR || p==WQ || p==BQ) {
        static const std::array<std::array<int,2>,4> d{{ {{-1,0}},{{1,0}},{{0,-1}},{{0,1}} }};
        std::for_each(d.begin(), d.end(), [&](const auto &o){
            for(int rr=r+o[0], cc=c+o[1]; add(rr,cc); rr+=o[0], cc+=o[1]){}
        });
        if (p==WR || p==BR) {}
    }
    if (p==WK || p==BK) {
        static const std::array<std::array<int,2>,8> d{{
            {{-1,-1}},{{-1,0}},{{-1,1}},{{0,-1}},{{0,1}},{{1,-1}},{{1,0}},{{1,1}}
        }};
        std::for_each(d.begin(), d.end(), [&](const auto &o){ add(r+o[0], c+o[1]); });
        // castling
        if(col==White && !m_whiteKingMoved){
            if(!m_whiteRightRookMoved && pieceAt(7,5)==Empty && pieceAt(7,6)==Empty && !isSquareAttacked(7,4,Black) && !isSquareAttacked(7,5,Black) && !isSquareAttacked(7,6,Black))
                res.append(QPoint(7,6));
            if(!m_whiteLeftRookMoved && pieceAt(7,3)==Empty && pieceAt(7,2)==Empty && pieceAt(7,1)==Empty && !isSquareAttacked(7,4,Black) && !isSquareAttacked(7,3,Black) && !isSquareAttacked(7,2,Black))
                res.append(QPoint(7,2));
        }
        if(col==Black && !m_blackKingMoved){
            if(!m_blackRightRookMoved && pieceAt(0,5)==Empty && pieceAt(0,6)==Empty && !isSquareAttacked(0,4,White) && !isSquareAttacked(0,5,White) && !isSquareAttacked(0,6,White))
                res.append(QPoint(0,6));
            if(!m_blackLeftRookMoved && pieceAt(0,3)==Empty && pieceAt(0,2)==Empty && pieceAt(0,1)==Empty && !isSquareAttacked(0,4,White) && !isSquareAttacked(0,3,White) && !isSquareAttacked(0,2,White))
                res.append(QPoint(0,2));
        }
    }

    QVector<QPoint> final;
    std::copy_if(res.cbegin(), res.cend(), std::back_inserter(final), [&](const QPoint &m){
        ChessBoard tmp = *this;
        int fi=r*8+c;
        int ti=m.x()*8+m.y();
        tmp.m_board[ti]=tmp.m_board[fi];
        tmp.m_board[fi]=Empty;
        return !tmp.isInCheck(col);
    });
    return final;
}

bool ChessBoard::isSquareAttacked(int r,int c,Color by) const
{
    int dir = (by==White)?-1:1;
    // pawn attacks
    if(r-dir>=0 && r-dir<8){
        if(c>0 && pieceAt(r-dir,c-1)==(by==White?WP:BP)) return true;
        if(c<7 && pieceAt(r-dir,c+1)==(by==White?WP:BP)) return true;
    }
    // knight attacks
    static const std::array<std::array<int,2>,8> nd{{
        {{-2,-1}},{{-2,1}},{{-1,-2}},{{-1,2}},{{1,-2}},{{1,2}},{{2,-1}},{{2,1}}
    }};
    if(std::any_of(nd.begin(), nd.end(), [&](const auto &o){
            int rr=r+o[0], cc=c+o[1];
            return rr>=0&&rr<8&&cc>=0&&cc<8 && pieceAt(rr,cc)==(by==White?WN:BN);
        }))
        return true;
    // bishop/queen attacks
    static const std::array<std::array<int,2>,4> bd{{ {{-1,-1}},{{-1,1}},{{1,-1}},{{1,1}} }};
    auto rayAttack = [&](int dr,int dc,const std::array<Piece,4> &pieces){
        for(int rr=r+dr,cc=c+dc; rr>=0&&rr<8&&cc>=0&&cc<8; rr+=dr,cc+=dc){
            Piece t=pieceAt(rr,cc);
            if(t!=Empty){
                if(pieceColor(t)==by && std::find(pieces.begin(), pieces.end(), t)!=pieces.end())
                    return true;
                break;
            }
        }
        return false;
    };
    if(std::any_of(bd.begin(), bd.end(), [&](const auto &o){
            return rayAttack(o[0], o[1], {BB,BQ,WB,WQ});
        }))
        return true;
    // rook/queen attacks
    static const std::array<std::array<int,2>,4> rd{{ {{-1,0}},{{1,0}},{{0,-1}},{{0,1}} }};
    if(std::any_of(rd.begin(), rd.end(), [&](const auto &o){
            return rayAttack(o[0], o[1], {BR,BQ,WR,WQ});
        }))
        return true;
    // king attacks
    static const std::array<std::array<int,2>,8> kd{{
        {{-1,-1}},{{-1,0}},{{-1,1}},{{0,-1}},{{0,1}},{{1,-1}},{{1,0}},{{1,1}}
    }};
    if(std::any_of(kd.begin(), kd.end(), [&](const auto &o){
            int rr=r+o[0], cc=c+o[1];
            return rr>=0&&rr<8&&cc>=0&&cc<8 && pieceAt(rr,cc)==(by==White?WK:BK);
        }))
        return true;
    return false;
}

bool ChessBoard::isInCheck(Color c) const
{
    auto it = std::find_if(m_board.cbegin(), m_board.cend(), [&](Piece p){
        return p == (c==White?WK:BK);
    });
    if(it == m_board.cend())
        return false;
    int idx = std::distance(m_board.cbegin(), it);
    int r = idx / 8;
    int col = idx % 8;
    return isSquareAttacked(r, col, c==White?Black:White);
}

bool ChessBoard::hasMoves(Color c) const
{
    int idx = 0;
    return std::any_of(m_board.cbegin(), m_board.cend(), [&](Piece p) mutable {
        int r = idx / 8;
        int col = idx % 8;
        ++idx;
        if(p!=Empty && pieceColor(p)==c){
            QString pos = QString("%1%2").arg(QChar('a'+col)).arg(8-r);
            return !legalMoves(pos).isEmpty();
        }
        return false;
    });
}

QString ChessBoard::toFen() const
{
    static const std::array<char, 13> pieceChars{{
        '1','P','R','N','B','Q','K','p','r','n','b','q','k'
    }};

    struct FenAcc { QString fen; int empty = 0; int idx = 0; };

    auto res = std::accumulate(m_board.begin(), m_board.end(), FenAcc{},
        [&](FenAcc a, Piece p){
            if(p==Empty){
                ++a.empty;
            }else{
                if(a.empty>0){ a.fen += QString::number(a.empty); a.empty = 0; }
                a.fen += pieceChars[static_cast<int>(p)];
            }
            ++a.idx;
            if(a.idx % 8 == 0){
                if(a.empty>0){ a.fen += QString::number(a.empty); a.empty = 0; }
                if(a.idx != 64) a.fen += '/';
            }
            return a;
        });

    QString fen = res.fen;
    fen+=' ';
    fen+=(m_turn==White?'w':'b');
    fen+=' ';
    QString rights;
    if(!m_whiteKingMoved && !m_whiteRightRookMoved) rights+="K";
    if(!m_whiteKingMoved && !m_whiteLeftRookMoved) rights+="Q";
    if(!m_blackKingMoved && !m_blackRightRookMoved) rights+="k";
    if(!m_blackKingMoved && !m_blackLeftRookMoved) rights+="q";
    if(rights.isEmpty()) rights="-";
    fen+=rights;
    fen+=' ';
    if(m_enPassant.x()!=-1)
        fen+=posToStr(m_enPassant.x(),m_enPassant.y());
    else
        fen+="-";
    fen+=" 0 ";
    fen+=QString::number(m_history.size()/2 + 1);
    return fen;
}
