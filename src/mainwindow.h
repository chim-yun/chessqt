#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "boardview.h"
#include <QGraphicsScene>
#include <QTimer>
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

private:
    enum Mode { Off, Offline, VsAi };
    Mode m_mode = Off;
    QString m_player;
    ChessBoard m_board;
    BoardView *m_view;
    QGraphicsScene *m_scene;
    QTimer m_timer;
    int m_whiteTime = 600; // 10 minutes
    int m_blackTime = 600;
};

#endif // MAINWINDOW_H
