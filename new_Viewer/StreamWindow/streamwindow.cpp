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

QVNCViewer *StreamViewerWindow::getViewer() const
{
    return ui->viewer_widget;
}

void StreamViewerWindow::closeEvent(QCloseEvent *event)
{
    ui->viewer_widget->stopFrameBufferUpdate();
    QMainWindow* p = static_cast<QMainWindow*>(this->parent());
    this->setVisible(false);
    p->setVisible(true);
    p->setFocus();
}
