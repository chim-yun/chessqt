#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Login login;
    if (!login.exec())
        return 0;

    MainWindow w(login.username());
    w.show();
    return app.exec();
}
