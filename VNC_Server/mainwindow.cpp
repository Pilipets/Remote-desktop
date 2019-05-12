#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    server = new QVNCServer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QMessageBox * infoWindow = new QMessageBox (this);
    infoWindow->setText(QString("Your IP: %1\nPort: %2").arg(server->getIP(),QString::number(server->getPort())));
    infoWindow->show();
}
