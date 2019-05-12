#ifndef QVNCSERVER_H
#define QVNCSERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include<QDataStream>
#include<QByteArray>
#include <QScreen>
#include <QTimer>

class QRfbEncoder;
#include "qvnc_connection.h"
class QVNCServer : public QObject
{
    Q_OBJECT
public:
    QVNCServer();
    ~QVNCServer();

    inline bool isConnected() const { return state == Connected; }
    inline QTcpSocket* clientSocket() const { return client; }
    inline int clientBytesPerPixel() const {
        return pixelFormat.bitsPerPixel / 8;
    }
private slots:
    void newConnection();
    void readClient();
    void discardClient();
    void checkUpdate();
private:

    void init(quint16 port);
    void fillNetworkInfo(quint16 port);
    void frameBufferUpdateRequest();
private:
    enum ClientMsg {FramebufferUpdateRequest = 3};

    enum ServerMsg { FramebufferUpdate = 0};

    enum ClientState { Unconnected, Protocol, Init, Connected };
    ClientState state;

    QTcpServer* serverSocket;
    QTcpSocket* client;
    QRfbPixelFormat pixelFormat;
    QRfbEncoder *encoder;

    QTimer *timer;
    int refreshRate;

    QString m_ip;
    quint16 m_port;
    bool handleMsg;
    quint8 msgType;
    bool wantUpdate;

public:
    inline QString getIP(){return m_ip;}
    inline quint16 getPort() {return  m_port;}
};
#endif // QVNCSERVER_H
