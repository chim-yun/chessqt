#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsRectItem>
#include <QPixmap>
#include "boardview.h"

MainWindow::MainWindow(const QString &user, QWidget *parent)
    : QMainWindow(parent), m_player(user)
{
    setWindowTitle("Chess - " + user);
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    auto *playOffline = new QPushButton("Offline 2 Players", this);
    auto *playAi = new QPushButton("Play vs AI", this);
    layout->addWidget(playOffline);
    layout->addWidget(playAi);
    setCentralWidget(central);

    connect(playOffline, &QPushButton::clicked, this, &MainWindow::chooseOffline);
    connect(playAi, &QPushButton::clicked, this, &MainWindow::chooseVsAi);

    m_scene = new QGraphicsScene(this);
    m_view = new BoardView(m_scene, &m_board, this);
    m_scene->setSceneRect(0,0,400,400);
}

void MainWindow::chooseOffline()
{
    m_mode = Offline;
    startGame();
}

void MainWindow::chooseVsAi()
{
    m_mode = VsAi;
    startGame();
}

void MainWindow::startGame()
{
    m_whiteTime = 600;
    m_blackTime = 600;
    m_board.reset();

    m_scene->clear();
    setCentralWidget(m_view);
    redrawBoard();

    m_timer.start(1000);
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(m_view, &BoardView::boardChanged, this, &MainWindow::redrawBoard);
}

void MainWindow::updateTimer()
{
    if (m_board.currentColor()==ChessBoard::White)
        --m_whiteTime;
    else
        --m_blackTime;
    if (m_whiteTime<=0 || m_blackTime<=0) {
        QMessageBox::information(this, "Time", m_whiteTime<=0?"Black wins":"White wins");
        m_timer.stop();
        disconnect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
        return;
    }
}

void MainWindow::redrawBoard()
{
    m_scene->clear();
    for (int r=0;r<8;++r) {
        for (int c=0;c<8;++c) {
            m_scene->addRect(c*50,r*50,50,50,QPen(),(r+c)%2?QBrush(Qt::gray):QBrush(Qt::white));
            ChessBoard::Piece p = m_board.pieceAt(r,c);
            if (p!=ChessBoard::Empty) {
                QString name;
                switch (p) {
                case ChessBoard::WP: name="pawn_w"; break;
                case ChessBoard::WR: name="rook_w"; break;
                case ChessBoard::WN: name="knight_w"; break;
                case ChessBoard::WB: name="bishop_w"; break;
                case ChessBoard::WQ: name="queen_w"; break;
                case ChessBoard::WK: name="king_w"; break;
                case ChessBoard::BP: name="pawn_b"; break;
                case ChessBoard::BR: name="rook_b"; break;
                case ChessBoard::BN: name="knight_b"; break;
                case ChessBoard::BB: name="bishop_b"; break;
                case ChessBoard::BQ: name="queen_b"; break;
                case ChessBoard::BK: name="king_b"; break;
                default: break;
                }
                QPixmap pix(":/images/"+name+".png");
                m_scene->addPixmap(pix.scaled(50,50))->setPos(c*50,r*50);
            }
        }
    }
}
