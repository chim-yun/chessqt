#include "login.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

Login::Login(QWidget *parent)
    : QDialog(parent)
{
    setupDb();

    setWindowTitle("Login");
    auto *layout = new QVBoxLayout(this);
    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText("Username");
    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText("Password");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_loginBtn = new QPushButton("Login", this);
    m_signBtn = new QPushButton("Sign up", this);
    connect(m_loginBtn, &QPushButton::clicked, this, &Login::logIn);
    connect(m_signBtn, &QPushButton::clicked, this, &Login::signIn);
    layout->addWidget(m_userEdit);
    layout->addWidget(m_passEdit);
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_signBtn);
    layout->addLayout(btnLayout);
}

void Login::setupDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("players.db");
    if (!db.open())
        QMessageBox::critical(this, "DB", "Failed to open database");
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users(name TEXT PRIMARY KEY, pass TEXT)");
}

void Login::signIn()
{
    QSqlQuery query;
    query.prepare("INSERT INTO users(name, pass) VALUES(?, ?)");
    query.addBindValue(m_userEdit->text());
    query.addBindValue(m_passEdit->text());
    if (!query.exec()) {
        QMessageBox::warning(this, "Sign in", "User exists");
        return;
    }
    QMessageBox::information(this, "Sign in", "Account created");
}

void Login::logIn()
{
    QSqlQuery query;
    query.prepare("SELECT pass FROM users WHERE name=?");
    query.addBindValue(m_userEdit->text());
    if (!query.exec() || !query.next() || query.value(0).toString() != m_passEdit->text()) {
        QMessageBox::warning(this, "Login", "Invalid credentials");
        return;
    }
    m_username = m_userEdit->text();
    accept();
}

