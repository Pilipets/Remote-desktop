#include "serverstartwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
ServerStartWindow::ServerStartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerStartWindow)
{
    ui->setupUi(this);
    server = new QVncServer(QGuiApplication::primaryScreen());
}

ServerStartWindow::~ServerStartWindow()
{
    delete ui;
    delete server;
}
