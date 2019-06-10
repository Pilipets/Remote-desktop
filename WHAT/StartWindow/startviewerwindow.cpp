#include "startviewerwindow.h"
#include "ui_startviewerwindow.h"

StartViewerWindow::StartViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartViewerWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
}

StartViewerWindow::~StartViewerWindow()
{
    delete ui;
}

void StartViewerWindow::on_connectBtn_clicked()
{
    ui->statusBar->showMessage("Connecting to the " + ui->addressLineEdit->text());
}

void StartViewerWindow::on_disconnectBtn_clicked()
{
    ui->statusBar->showMessage("Disconnecting from the host");
}
