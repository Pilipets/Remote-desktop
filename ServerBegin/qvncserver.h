#ifndef QVNCSERVER_H
#define QVNCSERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include<QDataStream>
#include<QByteArray>
#include <QScreen>
#include "vnc_server_helper.h"

class QVNCServer : public QObject
{
    Q_OBJECT
public:
    QVNCServer();
    QVNCServer(QScreen* screen);
    ~QVNCServer();
private slots:
    void newConnection();
    void readClient();
    void discardClient();
private:
    void init(quint16 port);
private:
    enum ClientState { Unconnected, Protocol, Init, Connected };
    ClientState state;

    QTcpServer* serverSocket;
    QTcpSocket* client;
    QScreen *qvnc_screen;

};

#endif // QVNCSERVER_H
