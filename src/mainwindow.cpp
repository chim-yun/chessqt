#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QRandomGenerator>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include "boardview.h"

MainWindow::MainWindow(const QString &user, QWidget *parent)
    : QMainWindow(parent), m_player(user)
{
    setWindowTitle("Chess - " + user);
    showMenu();

    m_scene = new QGraphicsScene(this);
    m_view = new BoardView(m_scene, &m_board, this);
    m_scene->setSceneRect(0,0,400,400);
}

void MainWindow::showMenu()
{
    if(m_menu)
        m_menu->deleteLater();
    m_menu = new QWidget(this);
    auto *layout = new QVBoxLayout(m_menu);
    auto *playOffline = new QPushButton("Offline 2 Players", m_menu);
    auto *playAi = new QPushButton("Play vs AI", m_menu);
    layout->addWidget(playOffline);
    layout->addWidget(playAi);
    setCentralWidget(m_menu);
    connect(playOffline, &QPushButton::clicked, this, &MainWindow::chooseOffline);
    connect(playAi, &QPushButton::clicked, this, &MainWindow::chooseVsAi);
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
        if(!m_net){
            m_net = new QNetworkAccessManager(this);
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
        disconnect(m_view, &BoardView::boardChanged, this, &MainWindow::onBoardChange);
        disconnect(m_view, &BoardView::highlightChanged, this, &MainWindow::setHighlight);
        showMenu();
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
    if(!m_net) return;
    QUrl url("https://stockfish.online/api/s/v2.php");
    QUrlQuery query;
    query.addQueryItem("fen", m_board.toFen());
    query.addQueryItem("depth", "12");
    url.setQuery(query);
    QNetworkRequest req(url);
    auto reply = m_net->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ handleAiReply(reply); });
}

void MainWindow::handleAiReply(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    reply->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if(!doc.isObject()) return;
    QString best = doc.object().value("bestmove").toString();
    if(best.startsWith("bestmove"))
        best = best.split(' ').value(1);
    if(best.size() >= 4){
        QString from = best.mid(0,2);
        QString to = best.mid(2,2);
        m_board.move(from,to);
        m_view->clearSelection();
        redrawBoard();
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
        m_timer.stop();
        disconnect(&m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
        disconnect(m_view, &BoardView::boardChanged, this, &MainWindow::onBoardChange);
        disconnect(m_view, &BoardView::highlightChanged, this, &MainWindow::setHighlight);
        showMenu();
    }
}
