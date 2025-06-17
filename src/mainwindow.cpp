#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QRandomGenerator>
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
    QStringList opts{"White","Black","Random"};
    bool ok=false;
    QString choice = QInputDialog::getItem(this,"Play vs AI","Select side",opts,0,false,&ok);
    if(!ok) return;
    if(choice=="Random")
        m_playerColor = (QRandomGenerator::global()->bounded(2)==0)?ChessBoard::White:ChessBoard::Black;
    else
        m_playerColor = (choice=="White")?ChessBoard::White:ChessBoard::Black;
    m_mode = VsAi;
    startGame();
    if(m_playerColor==ChessBoard::Black)
        requestAiMove();
}

void MainWindow::startGame()
{
    m_whiteTime = 600;
    m_blackTime = 600;
    m_board.reset();
    m_highlight.clear();

    m_scene->clear();
    setCentralWidget(m_view);
    redrawBoard();

    m_timer.start(1000);
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(m_view, &BoardView::boardChanged, this, &MainWindow::onBoardChange);
    connect(m_view, &BoardView::highlightChanged, this, &MainWindow::setHighlight);

    if(m_mode==VsAi){
        if(!m_engine){
            m_engine = new QProcess(this);
            m_engine->start("stockfish");
            m_engine->write("uci\n");
            m_engine->write("isready\n");
            m_engine->waitForReadyRead(3000);
            m_engine->readAll();
            connect(m_engine,&QProcess::readyRead,this,&MainWindow::readAiMove);
        }
    }
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
            QBrush brush = ((r+c)%2)?QBrush(Qt::gray):QBrush(Qt::white);
            for(const QPoint &p : m_highlight){ if(p.x()==r && p.y()==c){ brush = QBrush(Qt::yellow); break; } }
            m_scene->addRect(c*50,r*50,50,50,QPen(),brush);
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
                if(pix.isNull())
                    pix.load("../assets/"+name+".png");
                m_scene->addPixmap(pix.scaled(50,50))->setPos(c*50,r*50);
            }
        }
    }
}

void MainWindow::onBoardChange()
{
    redrawBoard();
    checkGameOver();
    if(m_mode==VsAi && m_board.currentColor()!=m_playerColor)
        requestAiMove();
}

void MainWindow::setHighlight(const QVector<QPoint> &moves)
{
    m_highlight = moves;
    redrawBoard();
}

void MainWindow::requestAiMove()
{
    if(!m_engine) return;
    QString cmd = "position startpos";
    auto hist = m_board.history();
    if(!hist.isEmpty())
        cmd += " moves " + hist.join(' ');
    m_engine->write(cmd.toUtf8()+"\n");
    m_engine->write("go movetime 1000\n");
}

void MainWindow::readAiMove()
{
    while(m_engine->canReadLine()){
        QByteArray line = m_engine->readLine();
        if(line.startsWith("bestmove")){
            QList<QByteArray> parts = line.split(' ');
            if(parts.size()>=2){
                QString mv = parts[1];
                QString from = mv.mid(0,2);
                QString to = mv.mid(2,2);
                m_board.move(from,to);
                m_view->clearSelection();
                redrawBoard();
            }
        }
    }
}

void MainWindow::checkGameOver()
{
    bool w=false,b=false;
    for(int r=0;r<8;++r)
        for(int c=0;c<8;++c){
            ChessBoard::Piece p=m_board.pieceAt(r,c);
            if(p==ChessBoard::WK) w=true;
            if(p==ChessBoard::BK) b=true;
        }
    if(!w||!b){
        QMessageBox::information(this,"Game Over",!w?"Black wins":"White wins");
        m_timer.stop();
        setCentralWidget(nullptr);
    }
}
