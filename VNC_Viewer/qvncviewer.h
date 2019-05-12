#ifndef QVNCVIEWER_H
#define QVNCVIEWER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include "qvnc_connection.h"
#include <QImage>
#include <QWidget>
#include <QtCore>
class QVNCViewer : public QWidget
{
    Q_OBJECT
public:
    explicit QVNCViewer(QWidget *parent);
    ~QVNCViewer() override;

    bool connectToVncServer(QString ip, quint16 port);
    void disconnectFromVncServer();
    void startFrameBufferUpdate()
    {
        connect(this, SIGNAL(frameBufferUpdated()), this, SLOT(sendFrameBufferUpdateRequest()));
        sendFrameBufferUpdateRequest();
    }

    void stopFrameBufferUpdate()
    {
        disconnect(this, SIGNAL(frameBufferUpdated()), this, SLOT(sendFrameBufferUpdateRequest()));
    }

protected:
    void paintEvent(QPaintEvent *) override;
private slots:
    void onServerMessage();
    void sendFrameBufferUpdateRequest();
signals:
    void frameBufferUpdated();

private:
    enum ServerMsg { FramebufferUpdate = 0};
    bool isFrameBufferUpdating;

    QTcpSocket* server;

    quint16 frameBufferWidth;
    quint16 frameBufferHeight;
    QRfbPixelFormat pixelFormat;
    QImage screen;

};

#endif // QVNCVIEWER_H
