#ifndef QVNCVIEWER_H
#define QVNCVIEWER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include "qvnc_connection.h"
#include <QImage>
class QVNCViewer : public QObject
{
    Q_OBJECT
public:
    QVNCViewer();
    ~QVNCViewer();

    bool connectToVncSever(QString ip, quint16 port);
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
