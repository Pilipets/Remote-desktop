#ifndef STREAMVIEWERWINDOW_H
#define STREAMVIEWERWINDOW_H

#include <QMainWindow>
#include"qvncviewer.h"
namespace Ui {
class StreamViewerWindow;
}

class StreamViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StreamViewerWindow(QWidget *parent = nullptr);
    ~StreamViewerWindow();

    QVNCViewer *getViewer() const;

private:
    Ui::StreamViewerWindow *ui;

    void closeEvent (QCloseEvent *event) override;
};

#endif // STREAMVIEWERWINDOW_H
