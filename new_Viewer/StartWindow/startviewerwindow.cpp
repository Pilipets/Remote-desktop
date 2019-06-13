#include "startviewerwindow.h"
#include "ui_startviewerwindow.h"

StartViewerWindow::StartViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartViewerWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    streamWindow = new StreamViewerWindow(this);
}

StartViewerWindow::~StartViewerWindow()
{
    delete streamWindow;
    delete ui;
}

void StartViewerWindow::on_connectBtn_clicked()
{
    if(streamWindow->getViewer()->isConnected())
    {
        this->on_showScreenBtn_clicked();
        return;
    }
    QString address = ui->addressLineEdit->text();
    QStringList parsed_list = address.split(':');
    QString ip = parsed_list[0];
    quint16 port = parsed_list[1].toInt();

    ui->statusBar->showMessage("Connecting to the " + address, 3000);
    if(streamWindow->getViewer()->connectToVncServer(ip,port)){
        ui->statusBar->showMessage("Connected to the " + address, 3000);
        switchToStreamWindow();
    }
    else {
        ui->statusBar->showMessage("Connection refused", 3000);
    }
}

void StartViewerWindow::on_disconnectBtn_clicked()
{
    if(streamWindow->getViewer()->isConnected()){
        ui->statusBar->showMessage("Disconnecting from the host",3000);
        streamWindow->getViewer()->disconnectFromVncServer();
        ui->statusBar->showMessage("Disconnected from the server",3000);
    }
    else{
        ui->statusBar->showMessage("You are not connected to the server", 3000);
    }
}


void StartViewerWindow::on_showScreenBtn_clicked()
{
    if(streamWindow->getViewer()->isConnected())
    {
        streamWindow->getViewer()->startFrameBufferUpdate();
        switchToStreamWindow();
    }
    else {
        ui->statusBar->showMessage("You are not connected to the server", 3000);
    }
}

void StartViewerWindow::switchToStreamWindow()
{
    this->hide();
    streamWindow->show();
    streamWindow->getViewer()->setFocus();
    streamWindow->getViewer()->setMouseTracking(true);
}
