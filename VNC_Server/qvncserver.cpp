#include "qvncserver.h"
#include <QDebug>
#include <QtEndian>
#include <QNetworkInterface>

QVNCServer::QVNCServer() : client(nullptr)
{
    init(5900);
}

void QVNCServer::init(quint16 port)
{
    qDebug() << "QVNCServer::init" << port;
    state = Unconnected;

    serverSocket = new QTcpServer(this);
    if (!serverSocket->listen(QHostAddress::Any, port))
        qDebug() << "QVNCServer could not connect:" << serverSocket->errorString();
    else
        qDebug("QVNCServer created on port %d", port);

    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(newConnection()));
    fillNetworkInfo(port);
}

void QVNCServer::fillNetworkInfo(quint16 port)
{
    m_port = port;
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
            m_ip = address.toString();
    }
}

void QVNCServer::frameBufferUpdateRequest()
{
    QRfbFrameBufferUpdateRequest ev;

    if (ev.read(client)) {
        if (!ev.incremental) {
            QRect r(ev.rect.x, ev.rect.y, ev.rect.w, ev.rect.h);
            r.translate(qvnc_screen->offset());
            qvnc_screen->d_ptr->setDirty(r, true);
        }
        wantUpdate = true;
        checkUpdate();
        handleMsg = false;
    }
}
void QVNCServer::newConnection()
{
    qDebug() << "new Connection registered\n";
    //new connection registered from serverSocket
    if (client)
        delete client;

    client = serverSocket->nextPendingConnection();
    connect(client,SIGNAL(readyRead()),this,SLOT(readClient()));
    connect(client,SIGNAL(disconnected()),this,SLOT(discardClient()));

    // send protocol version
    const char *proto = "RFB 003.008\n";
    client->write(proto, 12);
    state = Protocol;

    qDebug() << "RFB protocol has send\n";
}

void QVNCServer::readClient()
{
    switch (state) {
    case Protocol:
        //read Protocol from client
            if (client->bytesAvailable() >= 12) {
                char proto[13];
                client->read(proto, 12);
                proto[12] = '\0';
                qDebug("Client protocol version %s", proto);

                // No authentication
                //quint32 auth = htonl(1);
                quint32 auth = qToBigEndian(1);
                client->write((char *)&auth, sizeof(auth));
                state = Init;
            }
            break;
    case Init:
        if (client->bytesAvailable() >= 1) {
            quint8 shared;
            client->read((char *) &shared, 1);

            // Server Init msg
            QRfbServerInit sim;
            QRfbPixelFormat &format = sim.format;
            //screen depth = 32:
            format.bitsPerPixel = 32;
            format.depth = 32;
            format.bigEndian = 0;
            format.trueColor = true;
            format.redBits = 8;
            format.greenBits = 8;
            format.blueBits = 8;
            format.redShift = 16;
            format.greenShift = 8;
            format.blueShift = 0;

            sim.width = 480;//qvnc_screen->geometry().width();
            sim.height = 720;//qvnc_screen->geometry().height();
            sim.setName("Qt MAC VNC Server");
            sim.write(client);
            state = Connected;
        }
        break;
    case Connected:
        do {
            if (!handleMsg) {
                client->read((char *)&msgType, 1);
                handleMsg = true;
            }
            if (handleMsg) {
                switch (msgType ) {
                case FramebufferUpdateRequest:
                    frameBufferUpdateRequest();
                    break;
                 default:
                    qDebug("Unknown message type: %d", (int)msgType);
                    handleMsg = false;
                }
            }
        } while (!handleMsg && client->bytesAvailable());
        break;
    default:
        break;
    }
}

void QVNCServer::discardClient()
{
    state = Unconnected;
    qDebug() << "Client has disconnected\n";
}
QVNCServer::~QVNCServer()
{
    delete client;
    client = nullptr;
}
