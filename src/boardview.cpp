#include "boardview.h"
#include <QMouseEvent>
#include <QGraphicsPixmapItem>
#include "utils.h"

BoardView::BoardView(QGraphicsScene *scene, ChessBoard *board, QWidget *parent)
    : QGraphicsView(scene, parent), m_board(board)
{
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = mapToScene(event->pos());
    int c = pos.x()/50;
    int r = pos.y()/50;
    if (c<0||c>=8||r<0||r>=8) return;
    QString coord = posToStr(r,c);
    if (m_selected.isEmpty()) {
        ChessBoard::Piece p = m_board->pieceAt(r,c);
        if (p!=ChessBoard::Empty && m_board->pieceColor(p)==m_board->currentColor()) {
            m_selected = coord;
            m_moves = m_board->legalMoves(coord);
            emit highlightChanged(m_moves);
        }
    } else {
        if (m_board->move(m_selected, coord)) {
            m_selected.clear();
            m_moves.clear();
            emit boardChanged();
            emit highlightChanged({});
        } else {
            m_selected.clear();
            m_moves.clear();
            emit highlightChanged({});
        }
    }
}

void BoardView::clearSelection()
{
    m_selected.clear();
    m_moves.clear();
    emit highlightChanged({});
}
