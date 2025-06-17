#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "boardview.h"
#include <QGraphicsScene>
#include <QTimer>
#include <QProcess>
#include <QPushButton>
#include <QByteArray>
#include <QVector>
#include <QPoint>
#include <QLabel>
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
    void requestAiMove();
    void handleAiOutput();
    void resignGame();
    void onBoardChange();
    void checkGameOver();

private:
    void showMenu();
    void endGame();
    void updateTimerDisplay();
    void startAiEngine();

private:
    enum Mode { Off, Offline, VsAi };
    Mode m_mode = Off;
    QString m_player;
    ChessBoard m_board;
    BoardView *m_view;
    QGraphicsScene *m_scene;
    QTimer m_timer;
    QVector<QPoint> m_highlight;
    QProcess *m_ai = nullptr;
    QByteArray m_aiBuffer;
    bool m_backToLogin = false;
    ChessBoard::Color m_playerColor = ChessBoard::White;
    int m_whiteTime = 600; // 10 minutes
    int m_blackTime = 600;
    QLabel *m_whiteLabel = nullptr;
    QLabel *m_blackLabel = nullptr;
    QPushButton *m_resignBtn = nullptr;

public:
    bool backToLoginRequested() const { return m_backToLogin; }
};

#endif // MAINWINDOW_H
