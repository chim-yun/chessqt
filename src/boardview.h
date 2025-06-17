#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QGraphicsView>
#include <QVector>
#include "chessboard.h"

class BoardView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit BoardView(QGraphicsScene *scene, ChessBoard *board, QWidget *parent=nullptr);

    void setVsAiMode(bool vsAi) { m_vsAi = vsAi; }
    void setPlayerColor(ChessBoard::Color color) { m_playerColor = color; }

signals:
    void boardChanged();
    void highlightChanged(const QVector<QPoint> &moves);

protected:
    void mousePressEvent(QMouseEvent *event) override;

public:
    void clearSelection();

private:
    ChessBoard *m_board;
    QString m_selected;
    QVector<QPoint> m_moves;
    bool m_vsAi = false;
    ChessBoard::Color m_playerColor = ChessBoard::White;
};

#endif // BOARDVIEW_H
