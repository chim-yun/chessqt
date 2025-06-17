#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "boardview.h"
#include <QGraphicsScene>
#include <QTimer>
#include <QProcess>
#include <QVector>
#include <QPoint>
#include "chessboard.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &user, QWidget *parent = nullptr);

private slots:
    void startGame();
    void chooseVsAi();
    void chooseOffline();
    void updateTimer();
    void redrawBoard();
    void setHighlight(const QVector<QPoint> &moves);
    void readAiMove();
    void requestAiMove();
    void onBoardChange();
    void checkGameOver();

private:
    enum Mode { Off, Offline, VsAi };
    Mode m_mode = Off;
    QString m_player;
    ChessBoard m_board;
    BoardView *m_view;
    QGraphicsScene *m_scene;
    QTimer m_timer;
    QVector<QPoint> m_highlight;
    QProcess *m_engine = nullptr;
    ChessBoard::Color m_playerColor = ChessBoard::White;
    int m_whiteTime = 600; // 10 minutes
    int m_blackTime = 600;
};

#endif // MAINWINDOW_H
