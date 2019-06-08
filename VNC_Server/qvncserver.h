#ifndef QVNCSERVER_H
#define QVNCSERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include<QDataStream>
#include<QByteArray>
#include <QScreen>
#include <QTimer>
#include "qvnc_connection.h"

class QRfbEncoder;
class QVNCScreen;
class QVNCDirtyMap;
class QVNCServer : public QObject
{
    Q_OBJECT
public:
    QVNCServer(QVNCScreen* qvnc_screen);
    ~QVNCServer();

    void setDirty();
    inline bool isConnected() const { return state == Connected; }
    inline void setRefreshRate(int rate) { refreshRate = rate; }

    enum ClientMsg {FramebufferUpdateRequest = 3};
    enum ServerMsg { FramebufferUpdate = 0};

    inline int clientBytesPerPixel() const {
        return pixelFormat.bitsPerPixel / 8;
    }

    inline QVNCScreen* screen() const { return qvnc_screen; }
    inline QVNCDirtyMap* dirtyMap() const { return qvnc_screen->dirtyMap(); }

    inline QTcpSocket* clientSocket() const { return client; }
    QImage *screenImage() const;
private slots:
    void newConnection();
    void readClient();
    void discardClient();
    void checkUpdate();
private:
    void fillNetworkInfo(quint16 port);
    void frameBufferUpdateRequest();
private:
    void init(quint16 port);
    enum ClientState { Unconnected, Protocol, Init, Connected };

    QTimer *timer;
    QTcpServer* serverSocket;
    QTcpSocket* client;
    ClientState state;
    QVNCScreen *qvnc_screen;

    bool handleMsg;
    quint8 msgType;
    QRfbPixelFormat pixelFormat;
    bool wantUpdate;

    int refreshRate;
    QRfbEncoder *encoder;

    QString m_ip;
    quint16 m_port;
public:
    inline QString getIP(){return m_ip;}
    inline quint16 getPort() {return  m_port;}
};
#endif // QVNCSERVER_H
