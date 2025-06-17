#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QGraphicsView>
#include "chessboard.h"

class BoardView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit BoardView(QGraphicsScene *scene, ChessBoard *board, QWidget *parent=nullptr);

signals:
    void boardChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    ChessBoard *m_board;
    QString m_selected;
};

#endif // BOARDVIEW_H
