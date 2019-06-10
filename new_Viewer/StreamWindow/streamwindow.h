#ifndef STREAMVIEWERWINDOW_H
#define STREAMVIEWERWINDOW_H

#include <QMainWindow>

namespace Ui {
class StreamViewerWindow;
}

class StreamViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StreamViewerWindow(QWidget *parent = nullptr);
    ~StreamViewerWindow();


    void connect(const QString& address);

private:
    Ui::StreamViewerWindow *ui;

    void closeEvent (QCloseEvent *event) override;
};

#endif // STREAMVIEWERWINDOW_H
