#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QScreen* screen = QGuiApplication::primaryScreen();
    server = new QVNCServer(screen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete server;
}

void MainWindow::on_pushButton_clicked()
{
    QMessageBox * infoWindow = new QMessageBox (this);
    infoWindow->setText(QString("Your IP: %1\nPort: %2").arg(server->getIP(),QString::number(server->getPort())));
    infoWindow->show();
}
