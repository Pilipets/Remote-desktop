#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include "qvncviewer.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void connect_btn_clicked();
    void disconnect_btn_clicked();

private:
    Ui::MainWindow *ui;

    //QVNCViewer* viewer;
};

#endif // MAINWINDOW_H
