#include "chessboard.h"
#include "utils.h"
#include <numeric>

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
    bool legal=false;
    for (const QPoint &p : moves) {
        if (p.x()==tr && p.y()==tc) { legal=true; break; }
    }
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
        const int d[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
        for (auto &o:d) add(r+o[0],c+o[1]);
    }
    if (p==WB || p==BB || p==WQ || p==BQ) {
        const int d[4][2]={{-1,-1},{-1,1},{1,-1},{1,1}};
        for (auto &o:d){int rr=r+o[0],cc=c+o[1];while(add(rr,cc)){rr+=o[0];cc+=o[1];}}
        if (p==WB || p==BB) {}
    }
    if (p==WR || p==BR || p==WQ || p==BQ) {
        const int d[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
        for (auto &o:d){int rr=r+o[0],cc=c+o[1];while(add(rr,cc)){rr+=o[0];cc+=o[1];}}
        if (p==WR || p==BR) {}
    }
    if (p==WK || p==BK) {
        const int d[8][2]={{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
        for (auto &o:d) add(r+o[0],c+o[1]);
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
    for(const QPoint &m : res){
        ChessBoard tmp = *this;
        int fi=r*8+c;
        int ti=m.x()*8+m.y();
        tmp.m_board[ti]=tmp.m_board[fi];
        tmp.m_board[fi]=Empty;
        if(!tmp.isInCheck(col))
            final.append(m);
    }
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
    const int nd[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(auto &o:nd){int rr=r+o[0],cc=c+o[1];if(rr>=0&&rr<8&&cc>=0&&cc<8){Piece t=pieceAt(rr,cc);if(t==(by==White?WN:BN)) return true;}}
    // bishop/queen attacks
    const int bd[4][2]={{-1,-1},{-1,1},{1,-1},{1,1}};
    for(auto&o:bd){int rr=r+o[0],cc=c+o[1];while(rr>=0&&rr<8&&cc>=0&&cc<8){Piece t=pieceAt(rr,cc);if(t!=Empty){if(pieceColor(t)==by&&(t==BB||t==BQ||t==WB||t==WQ)) return true; break;}rr+=o[0];cc+=o[1];}}
    // rook/queen attacks
    const int rd[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
    for(auto&o:rd){int rr=r+o[0],cc=c+o[1];while(rr>=0&&rr<8&&cc>=0&&cc<8){Piece t=pieceAt(rr,cc);if(t!=Empty){if(pieceColor(t)==by&&(t==BR||t==BQ||t==WR||t==WQ)) return true; break;}rr+=o[0];cc+=o[1];}}
    // king attacks
    const int kd[8][2]={{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for(auto&o:kd){int rr=r+o[0],cc=c+o[1];if(rr>=0&&rr<8&&cc>=0&&cc<8){Piece t=pieceAt(rr,cc);if(t==(by==White?WK:BK)) return true;}}
    return false;
}

bool ChessBoard::isInCheck(Color c) const
{
    for(int r=0;r<8;++r)
        for(int col=0;col<8;++col){
            Piece p = pieceAt(r,col);
            if(p==(c==White?WK:BK))
                return isSquareAttacked(r,col,c==White?Black:White);
        }
    return false;
}

bool ChessBoard::hasMoves(Color c) const
{
    for(int r=0;r<8;++r)
        for(int col=0;col<8;++col){
            Piece p=pieceAt(r,col);
            if(p!=Empty && pieceColor(p)==c){
                QString pos = QString("%1%2").arg(QChar('a'+col)).arg(8-r);
                if(!legalMoves(pos).isEmpty())
                    return true;
            }
        }
    return false;
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
