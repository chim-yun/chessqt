#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    bool again;
    do {
        Login login;
        if (!login.exec())
            break;

        MainWindow w(login.username());
        w.show();
        app.exec();
        again = w.backToLoginRequested();
    } while(again);

    return 0;
}
