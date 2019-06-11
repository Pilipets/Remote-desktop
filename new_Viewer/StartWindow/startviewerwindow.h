#ifndef STARTVIEWERWINDOW_H
#define STARTVIEWERWINDOW_H

#include <QMainWindow>
#include "StreamWindow/streamwindow.h"
namespace Ui {
class StartViewerWindow;
}

class StartViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StartViewerWindow(QWidget *parent = nullptr);
    ~StartViewerWindow();

private slots:
    void on_connectBtn_clicked();

    void on_disconnectBtn_clicked();

    void on_showScreenBtn_clicked();

private:
    Ui::StartViewerWindow *ui;

    StreamViewerWindow* streamWindow;

    void switchToStreamWindow();
};

#endif // STARTVIEWERWINDOW_H
