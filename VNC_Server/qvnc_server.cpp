#include "qvnc_server.h"
#include "qvnc_client.h"
#include <QMetaObject>
#include <QPixmap>
#include <QDebug>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QScreen>


QVncServer::QVncServer(QScreen *screen, quint16 port):
    qvnc_screen(screen), m_port(port)
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}
void QVncServer::init()
{
    serverSocket = new QTcpServer();

    if (!serverSocket->listen(QHostAddress::Any, m_port))
        qWarning() << "QVncServer could not connect:" << serverSocket->errorString();
    else
        qWarning("QVncServer created on port %d", m_port);

    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(newConnection()));
}
QVncServer::~QVncServer()
{
    qDeleteAll(clients);
}

QImage QVncServer::screenImage() const
{
    return qvnc_screen->grabWindow(0).toImage();
}

void QVncServer::discardClient(QVncClient *client)
{
    qDebug() << "Client has disconnected\n";
    clients.removeOne(client);
    client->deleteLater();
}

void QVncServer::newConnection()
{
    QTcpSocket* clientSocket = serverSocket->nextPendingConnection();
    clients.append(new QVncClient(clientSocket, this));

    qDebug() << "new Connection from: " << clientSocket->localAddress();
}

