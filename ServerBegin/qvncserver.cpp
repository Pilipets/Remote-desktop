#include "qvncserver.h"
#include <QDebug>

QVNCServer::QVNCServer()
{
    init(5900);
}

QVNCServer::QVNCServer(QScreen *screen) : qvnc_screen(screen)
{
    init(5900);
}

void QVNCServer::newConnection()
{
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
                //quint32 auth = 1; //32bit

                //QByteArray packet;
                //QDataStream out(&packet, QIODevice::ReadOnly);
                //out.setByteOrder(QDataStream::BigEndian);
                //out << auth;
                //client->write(packet);

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
            switch (qvnc_screen->depth()) {
            case 32:
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
                break;

            case 24:
                format.bitsPerPixel = 24;
                format.depth = 24;
                format.bigEndian = 0;
                format.trueColor = true;
                format.redBits = 8;
                format.greenBits = 8;
                format.blueBits = 8;
                format.redShift = 16;
                format.greenShift = 8;
                format.blueShift = 0;
                break;

            case 18:
                format.bitsPerPixel = 24;
                format.depth = 18;
                format.bigEndian = 0;
                format.trueColor = true;
                format.redBits = 6;
                format.greenBits = 6;
                format.blueBits = 6;
                format.redShift = 12;
                format.greenShift = 6;
                format.blueShift = 0;
                break;

            case 16:
                format.bitsPerPixel = 16;
                format.depth = 16;
                format.bigEndian = 0;
                format.trueColor = true;
                format.redBits = 5;
                format.greenBits = 6;
                format.blueBits = 5;
                format.redShift = 11;
                format.greenShift = 5;
                format.blueShift = 0;
                break;

            case 15:
                format.bitsPerPixel = 16;
                format.depth = 15;
                format.bigEndian = 0;
                format.trueColor = true;
                format.redBits = 5;
                format.greenBits = 5;
                format.blueBits = 5;
                format.redShift = 10;
                format.greenShift = 5;
                format.blueShift = 0;
                break;

            case 12:
                format.bitsPerPixel = 16;
                format.depth = 12;
                format.bigEndian = 0;
                format.trueColor = true;
                format.redBits = 4;
                format.greenBits = 4;
                format.blueBits = 4;
                format.redShift = 8;
                format.greenShift = 4;
                format.blueShift = 0;
                break;

            case 8:
            case 4:
                format.bitsPerPixel = 8;
                format.depth = 8;
                format.bigEndian = 0;
                format.trueColor = false;
                format.redBits = 0;
                format.greenBits = 0;
                format.blueBits = 0;
                format.redShift = 0;
                format.greenShift = 0;
                format.blueShift = 0;
                break;

            default:
                qDebug("QVNC cannot drive depth %d", qvnc_screen->depth());
                discardClient();
                return;
            }
            sim.width = static_cast<quint16>(qvnc_screen->geometry().width());
            sim.height = static_cast<quint16>(qvnc_screen->geometry().height());
            sim.setName("Qt for MAC VNC Server");
            sim.write(client);

            state = Connected;
        }
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
}

QVNCServer::~QVNCServer()
{
    delete client;
    client = nullptr;
}
