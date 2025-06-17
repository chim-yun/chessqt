#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    while (true) {
        Login login;
        if (!login.exec())
            break;


    MainWindow w(login.username());
    w.show();
    return app.exec();

}
