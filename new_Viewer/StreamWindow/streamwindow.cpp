#include "streamwindow.h"
#include "ui_streamviewerwindow.h"

StreamViewerWindow::StreamViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StreamViewerWindow)
{
    ui->setupUi(this);
}

StreamViewerWindow::~StreamViewerWindow()
{
    delete ui;
}

void StreamViewerWindow::connect(const QString &address)
{
    QStringList parsed_list = address.split(':');
    QString ip = parsed_list[0];
    quint16 port = parsed_list[1].toInt();

    qDebug() << "Connect button pressed\n";
    qDebug() << ip << ": " << port << endl;

    if(ui->viewer_widget->connectToVncServer(ip,port))
    {
        QMainWindow* p = static_cast<QMainWindow*>(this->parent());
        p->setVisible(false);
        this->setVisible(true);
    }
}

void StreamViewerWindow::closeEvent(QCloseEvent *event)
{
    ui->viewer_widget->disconnectFromVncServer();
    QMainWindow* p = static_cast<QMainWindow*>(this->parent());
    this->setVisible(false);
    p->setVisible(true);
}
