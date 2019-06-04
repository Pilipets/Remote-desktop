#include "qvncviewer.h"
#include<QtNetwork/QHostAddress>
#include <QPainter>
QVNCViewer::QVNCViewer(QWidget *parent) : QWidget(parent)
{
    server = new QTcpSocket();
    isFrameBufferUpdating = true;

    connect(server, &QTcpSocket::disconnected, this, &QVNCViewer::disconnectFromVncServer);
}

QVNCViewer::~QVNCViewer()
{
    disconnectFromVncServer();
    delete server;
}

bool QVNCViewer::connectToVncServer(QString ip, quint16 port)
{
    qDebug() << "Trying connect to " << ip << port;
    server->connectToHost(QHostAddress(ip), port);
    if(server->waitForConnected(5000)){
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
    }
    else
    {
        qDebug() << "Not connected to " << ip;
        qDebug() << server->errorString();
        return false;
    }
    screen = QImage(frameBufferWidth, frameBufferHeight, QImage::Format_RGB32);
    startFrameBufferUpdate();
    return true;
}

void QVNCViewer::disconnectFromVncServer()
{

    disconnect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));
    server->disconnectFromHost();
    server->close();
}

void QVNCViewer::paintEvent(QPaintEvent *)
{
    if(screen.isNull())
    {
        screen = QImage(width(), height(), QImage::Format_RGB32);
        screen.fill(Qt::red);
    }

    QPainter painter;
    painter.begin(this);
    painter.drawImage(0, 0, screen.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    painter.end();
}

void QVNCViewer::onServerMessage()
{
    disconnect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));

    QByteArray response;
    int noOfRects;
    response = server->read(1);
    switch(response.at(0))
    {

    // ***************************************************************************************
    // ***************************** Frame Buffer Update *************************************
    // ***************************************************************************************
    case FramebufferUpdate:

        response = server->read(1); // padding
        response = server->read(2); // number of rectangles

        noOfRects = qMakeU16(response.at(0), response.at(1));

        for(int i=0; i<noOfRects; i++)
        {

            qApp->processEvents();
            response = server->read(2);
            int xPosition = qMakeU16(response.at(0), response.at(1));
            response = server->read(2);
            int yPosition = qMakeU16(response.at(0), response.at(1));
            response = server->read(2);
            int width = qMakeU16(response.at(0), response.at(1));
            response = server->read(2);
            int height = qMakeU16(response.at(0), response.at(1));
            response = server->read(4);
            int encodingType = qMakeU32(response.at(0), response.at(1), response.at(2), response.at(3));

            QImage image(width, height, QImage::Format_RGB32);

            if(encodingType == 0)
            {

                int noOfBytes = width * height * (pixelFormat.bitsPerPixel / 8);
                QByteArray pixelsData;

                do
                {
                    qApp->processEvents();
                    QByteArray temp = server->read(noOfBytes);
                    pixelsData.append(temp);
                    noOfBytes -= temp.size();
                }
                while(noOfBytes > 0);

                uchar* img_pointer = image.bits();

                int pixel_byte_cnt = 0;
                for(int i=0; i<height; i++)
                {
                    qApp->processEvents();

                    for(int j=0; j<width; j++)
                    {
                        // The order of the colors is BGR (not RGB)
                        img_pointer[0] = pixelsData.at(pixel_byte_cnt);
                        img_pointer[1] = pixelsData.at(pixel_byte_cnt+1);
                        img_pointer[2] = pixelsData.at(pixel_byte_cnt+2);
                        img_pointer[3] = pixelsData.at(pixel_byte_cnt+3);

                        pixel_byte_cnt += 4;
                        img_pointer += 4;
                    }
                }
            }

            QPainter painter(&screen);
            painter.drawImage(xPosition, yPosition, image);
            painter.end();

            repaint();
        }

        //emit frameBufferUpdated();
        break;

    }

    connect(server, SIGNAL(readyRead()), this, SLOT(onServerMessage()));
}

void QVNCViewer::sendFrameBufferUpdateRequest()
{
    qDebug() << "Sending Frame buffer Update Request\n";
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

