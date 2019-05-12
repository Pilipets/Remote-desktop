#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    viewer = new QVNCViewer();
    connect(ui->connect_btn, SIGNAL(clicked()),this, SLOT(connect_btn_clicked()));
    connect(ui->disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_btn_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete viewer;
}

void MainWindow::connect_btn_clicked()
{
    QString address = ui->adress_txt->text();
    QStringList parsed_list = address.split(':');
    QString ip = parsed_list[0];
    quint16 port = parsed_list[1].toInt();

    qDebug() << "Connect button pressed\n";
    qDebug() << ip << ": " << port << endl;

    viewer->connectToVncSever(ip,port);
    //viewer->startFrameBufferUpdate();
}

void MainWindow::disconnect_btn_clicked()
{
    viewer->disconnectFromVncServer();
}
