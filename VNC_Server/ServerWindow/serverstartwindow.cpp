#include "serverstartwindow.h"
#include "ui_mainwindow.h"

ServerStartWindow::ServerStartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerStartWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    server = new QVncServer(QGuiApplication::primaryScreen());
}

ServerStartWindow::~ServerStartWindow()
{
    delete ui;
    delete server;
}
