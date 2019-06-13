#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "VNCServer/qvnc_server.h"

namespace Ui {
class ServerStartWindow;
}

class ServerStartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerStartWindow(QWidget *parent = nullptr);
    ~ServerStartWindow();
private:
    Ui::ServerStartWindow *ui;
    QVncServer *server;
};

#endif // MAINWINDOW_H
