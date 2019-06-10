#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->viewer_widget->setFocus();
    ui->viewer_widget->setMouseTracking(true);

    connect(ui->connect_btn, SIGNAL(clicked()),this, SLOT(connect_btn_clicked()));
    connect(ui->disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_btn_clicked()));
}

MainWindow::~MainWindow()
{
    disconnect(ui->connect_btn, SIGNAL(clicked()),this, SLOT(connect_btn_clicked()));
    disconnect(ui->disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_btn_clicked()));
    delete ui;
}

void MainWindow::connect_btn_clicked()
{
    QString address = ui->adress_txt->text();
    QStringList parsed_list = address.split(':');
    QString ip = parsed_list[0];
    quint16 port = parsed_list[1].toInt();

    qDebug() << "Connect button pressed\n";
    qDebug() << ip << ": " << port << endl;

    ui->viewer_widget->connectToVncServer(ip,port);
    ui->viewer_widget->setFocus();
}

void MainWindow::disconnect_btn_clicked()
{
    qDebug() << "Disconnect button pressed\n";
    ui->viewer_widget->disconnectFromVncServer();
    ui->viewer_widget->setFocus();
}
