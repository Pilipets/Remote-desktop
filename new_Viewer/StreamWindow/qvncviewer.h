#ifndef QVNCVIEWER_H
#define QVNCVIEWER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include "NetworkHandlers/qvnc_connection.h"
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
    bool isConnected() {return m_state == Connected || m_state == Paused;}
    void startFrameBufferUpdate();

    void stopFrameBufferUpdate();
private:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void handleFrameBufferUpdate();
private slots:
    void readServer();
    void sendFrameBufferUpdateRequest();
signals:
    void frameBufferUpdated();

private:
    enum ServerMsg { FramebufferUpdate = 0};
    enum ClientState {
        Disconnected, Protocol, Authentication, Init, Connected, Paused};

    QTcpSocket* server;

    quint16 frameBufferWidth;
    quint16 frameBufferHeight;
    QImage screen;

    ClientState m_state;
    quint8 m_msgType;
    bool m_handleMsg;
    QRfbPixelFormat pixelFormat;
};

#endif // QVNCVIEWER_H
