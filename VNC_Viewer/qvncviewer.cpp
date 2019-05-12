#include "qvncviewer.h"
#include<QtNetwork/QHostAddress>
QVNCViewer::QVNCViewer()
{
    server = new QTcpSocket();
    isFrameBufferUpdating = true;
}

QVNCViewer::~QVNCViewer()
{
    disconnectFromVncServer();
}

bool QVNCViewer::connectToVncSever(QString ip, quint16 port)
{
    qDebug() << "Trying connect to " << ip << port;
    server->connectToHost(QHostAddress(ip), port);
    if(server->waitForConnected()){
        qDebug() << "Connected to Server\n";
        server->waitForReadyRead();

        char proto[13];
        server->read(proto, 12);
        proto[12] = '\0';
        qDebug("Server protocol version %s", proto);
        server->write("RFB 003.008\n");
        server->waitForReadyRead();

        // No authentication
        quint32 auth;
        server->read((char *)&auth, sizeof(auth));

        quint8 shared = 0;
        server->write((char *)&shared, sizeof(shared));

        server->waitForReadyRead();

        QRfbServerInit response;
        response.read(server);

        frameBufferWidth = response.width;
        frameBufferHeight = response.height;
        pixelFormat = response.format;

        qDebug() << "Width: " << frameBufferWidth;
        qDebug() << "Height: " << frameBufferHeight;

        // Pixel Format
        // ***************************
        qDebug() << "Bits per pixel: " << pixelFormat.bitsPerPixel;
        qDebug() << "Depth: " << pixelFormat.depth;
        qDebug() << "Big Endian: " << pixelFormat.bigEndian;
        qDebug() << "True Color: " << pixelFormat.trueColor;
        qDebug() << "Red Max: " << pixelFormat.redBits;
        qDebug() << "Green Max: " << pixelFormat.greenBits;
        qDebug() << "Blue Max: " << pixelFormat.blueBits;
        qDebug() << "Red Shift: " << pixelFormat.redShift;
        qDebug() << "Green Shift: " << pixelFormat.greenShift;
        qDebug() << "Blue Shift: " << pixelFormat.blueShift;

        qDebug() << "Name: " << response.name << "\n";
        screen = QImage(frameBufferWidth, frameBufferHeight, QImage::Format_RGB32);
        return true;
    }
    return  false;
}

void QVNCViewer::disconnectFromVncServer()
{
    server->close();
    server->disconnectFromHost();
    disconnect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));
}

void QVNCViewer::onServerMessage()
{
    //disconnect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));

    QByteArray response;
    response = server->read(1);
    quint8 mType = response.at(0);

    int noOfRects;
    switch(mType)
    {
    case FramebufferUpdate:
        break;
    default:
        break;
    }
}

void QVNCViewer::sendFrameBufferUpdateRequest()
{
    QByteArray frameBufferUpdateRequest(10, 0);
    frameBufferUpdateRequest[0] = 3; // message type must be 3
    frameBufferUpdateRequest[1] = 1; // incremental mode is zero for now (can help optimize the VNC client)
    frameBufferUpdateRequest[2] = 0;// x position
    frameBufferUpdateRequest[3] = 0;// x position
    frameBufferUpdateRequest[4] = 0;// y position
    frameBufferUpdateRequest[5] = 0;// y position

    frameBufferUpdateRequest[6] = (frameBufferWidth >> 8) & 0xFF; // width
    frameBufferUpdateRequest[7] = (frameBufferWidth >> 0) & 0xFF; // width
    frameBufferUpdateRequest[8] = (frameBufferHeight >> 8) & 0xFF; // height
    frameBufferUpdateRequest[9] = (frameBufferHeight >> 0) & 0xFF; // height

    server->write(frameBufferUpdateRequest);

    connect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));
}
