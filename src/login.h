#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QtSql>
#include <QLineEdit>
#include <QPushButton>
#include "mainwindow.h"

class Login : public QDialog
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    QString username() const { return m_username; }

private slots:
    void signIn();
    void logIn();

private:
    void setupDb();
    QString m_username;
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_signBtn;
};

#endif // LOGIN_H
