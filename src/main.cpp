#include <QtWidgets>
#include <array>

struct Piece { char symbol; bool white; };
using Board = std::array<std::array<Piece,8>,8>;

Board initBoard() {
    Board b{};
    auto place=[&](int row,int col,char sym,bool w){b[row][col]={sym,w};};
    for(int i=0;i<8;++i){place(1,i,'P',false); place(6,i,'P',true);}    
    const char pieces[]={'R','N','B','Q','K','B','N','R'};
    for(int i=0;i<8;++i){place(0,i,pieces[i],false); place(7,i,pieces[i],true);}    
    return b;
}

void updateTable(QTableWidget* table,const Board& b){
    for(int r=0;r<8;++r){
        for(int c=0;c<8;++c){
            char s=b[r][c].symbol;
            QString text = s?QString(s):"";
            auto item=new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(r,c,item);
        }
    }
}

bool showLogin(){
    QDialog d; d.setWindowTitle("Login");
    QVBoxLayout layout(&d);
    QLineEdit user; user.setPlaceholderText("User");
    QLineEdit pass; pass.setPlaceholderText("Password"); pass.setEchoMode(QLineEdit::Password);
    QDialogButtonBox box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&user); layout.addWidget(&pass); layout.addWidget(&box);
    QObject::connect(&box,&QDialogButtonBox::accepted,&d,&QDialog::accept);
    QObject::connect(&box,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    return d.exec()==QDialog::Accepted;
}

bool showMenu(){
    QDialog d; d.setWindowTitle("Menu");
    QVBoxLayout layout(&d);
    QPushButton start("Start Game");
    QPushButton quit("Quit");
    layout.addWidget(&start); layout.addWidget(&quit);
    QObject::connect(&start,&QPushButton::clicked,&d,&QDialog::accept);
    QObject::connect(&quit,&QPushButton::clicked,&d,&QDialog::reject);
    return d.exec()==QDialog::Accepted;
}

QMainWindow* createBoardWindow(){
    auto board=std::make_shared<Board>(initBoard());
    QMainWindow* w=new QMainWindow();
    QTableWidget* table=new QTableWidget(8,8);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setFixedSize(400,400);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    updateTable(table,*board);
    w->setCentralWidget(table);
    static std::pair<int,int> sel{-1,-1};
    QObject::connect(table,&QTableWidget::cellClicked,[=](int r,int c){
        if(sel.first==-1){ sel={r,c}; }
        else {
            (*board)[r][c]=(*board)[sel.first][sel.second];
            (*board)[sel.first][sel.second]={0,false};
            sel={-1,-1};
            updateTable(table,*board);
        }
    });
    return w;
}

int main(int argc,char** argv){
    QApplication app(argc,argv);
    if(!showLogin()) return 0;
    if(!showMenu()) return 0;
    QMainWindow* board=createBoardWindow();
    board->show();
    return app.exec();
}
