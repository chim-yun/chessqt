#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QLabel>
#include <QRandomGenerator>
#include <algorithm>
#include <ranges>
#include <QProcess>
#include <QCoreApplication>
#include "boardview.h"

MainWindow::MainWindow(const QString &user, QWidget *parent)
    : QMainWindow(parent), m_player(user)
{
    setWindowTitle("Chess - " + user);
    m_scene = new QGraphicsScene(this);
    m_view = new BoardView(m_scene, &m_board, this);
    m_scene->setSceneRect(0,0,400,400);

    m_whiteLabel = new QLabel(this);
    m_blackLabel = new QLabel(this);
    m_whiteLabel->setVisible(false);
    m_blackLabel->setVisible(false);

    showMenu();
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

    if(QWidget *old = centralWidget())
        old->deleteLater();
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_view);
    auto *timerLayout = new QHBoxLayout();
    timerLayout->addWidget(m_whiteLabel);
    timerLayout->addStretch();
    timerLayout->addWidget(m_blackLabel);
    layout->addLayout(timerLayout);
    setCentralWidget(central);
    m_view->show();
    redrawBoard();

    m_whiteLabel->setVisible(true);
    m_blackLabel->setVisible(true);
    updateTimerDisplay();

    m_timer.start(1000);
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(m_view, &BoardView::boardChanged, this, &MainWindow::onBoardChange);
    connect(m_view, &BoardView::highlightChanged, this, &MainWindow::setHighlight);

    if(m_mode==VsAi){
        if(!m_engine){
            m_engine = new QProcess(this);
            QString path = QCoreApplication::applicationDirPath()+"/../../stockfish/src/stockfish";
            m_engine->start(path);
            m_engine->write("uci\n");
        }
        connect(m_engine, &QProcess::readyReadStandardOutput, this, &MainWindow::handleEngineOutput);
    }

}

void MainWindow::updateTimer()
{
    if (m_board.currentColor()==ChessBoard::White)
        --m_whiteTime;
    else
        --m_blackTime;
    updateTimerDisplay();
    if (m_whiteTime<=0 || m_blackTime<=0) {
        QMessageBox::information(this, "Time", m_whiteTime<=0?"Black wins":"White wins");
        endGame();
        return;
    }
}

void MainWindow::redrawBoard()
{
    m_scene->clear();
    auto rng = std::views::iota(0,8);
    std::ranges::for_each(rng, [&](int r){
        std::ranges::for_each(rng, [&](int c){
            QBrush brush = ((r+c)%2)?QBrush(Qt::gray):QBrush(Qt::white);
            if(std::any_of(m_highlight.cbegin(), m_highlight.cend(), [&](const QPoint &p){ return p.x()==r && p.y()==c; }))
                brush = QBrush(Qt::yellow);
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
        });
    });
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
    QString fen = m_board.toFen();
    QByteArray cmd = "position fen " + fen.toUtf8() + "\n";
    m_engine->write(cmd);
    m_engine->write("go depth 12\n");
}

void MainWindow::handleEngineOutput()
{
    while(m_engine && m_engine->canReadLine()){
        QByteArray line = m_engine->readLine();
        if(line.startsWith("bestmove")){
            QList<QByteArray> parts = line.split(' ');
            if(parts.size() >= 2){
                QString best = parts.at(1);
                if(best.size() >= 4){
                    QString from = best.mid(0,2);
                    QString to = best.mid(2,2);
                    m_board.move(from,to);
                    m_view->clearSelection();
                    redrawBoard();
                    checkGameOver();
                }
            }
        }
    }
}

void MainWindow::checkGameOver()
{
    ChessBoard::Color cur = m_board.currentColor();
    if(!m_board.hasMoves(cur)){
        QString msg;
        if(m_board.isInCheck(cur))
            msg = (cur==ChessBoard::White)?"Black wins":"White wins";
        else
            msg = "Stalemate";
        QMessageBox::information(this,"Game Over",msg);
        endGame();
    }
}

void MainWindow::showMenu()
{
    if(QWidget *old = centralWidget())
        old->deleteLater();


    if(m_engine)
        m_engine->disconnect(this);

    // keep widgets alive when replacing the central widget
    m_view->setParent(this);
    m_whiteLabel->setParent(this);
    m_blackLabel->setParent(this);
    m_view->hide();
    m_whiteLabel->setVisible(false);
    m_blackLabel->setVisible(false);
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    auto *playOffline = new QPushButton("Offline 2 Players", this);
    auto *playAi = new QPushButton("Play vs AI", this);
    layout->addWidget(playOffline);
    layout->addWidget(playAi);
    setCentralWidget(central);

    connect(playOffline, &QPushButton::clicked, this, &MainWindow::chooseOffline);
    connect(playAi, &QPushButton::clicked, this, &MainWindow::chooseVsAi);
}

void MainWindow::endGame()
{
    m_timer.stop();
    disconnect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    disconnect(m_view, &BoardView::boardChanged, this, &MainWindow::onBoardChange);
    disconnect(m_view, &BoardView::highlightChanged, this, &MainWindow::setHighlight);
    m_highlight.clear();
    m_mode = Off;
    m_whiteLabel->setVisible(false);
    m_blackLabel->setVisible(false);
    if(m_engine){
        m_engine->write("quit\n");
        m_engine->waitForFinished(1000);
        delete m_engine;
        m_engine = nullptr;
    }
    showMenu();
}

void MainWindow::updateTimerDisplay()
{
    auto format=[&](int t){ return QString("%1:%2").arg(t/60,2,10,QChar('0')).arg(t%60,2,10,QChar('0')); };
    m_whiteLabel->setText("White: " + format(m_whiteTime));
    m_blackLabel->setText("Black: " + format(m_blackTime));
}
