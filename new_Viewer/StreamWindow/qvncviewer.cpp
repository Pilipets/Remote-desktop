#include "qvncviewer.h"
#include<QtNetwork/QHostAddress>
#include <QPainter>
#include <QKeyEvent>
#include <QImageReader>

QVNCViewer::QVNCViewer(QWidget *parent):
    QWidget(parent), m_state(Disconnected), m_msgType(0), m_handleMsg(false),
    frameBufferWidth(0), frameBufferHeight(0)
{
    server = new QTcpSocket();

    connect(server, &QTcpSocket::disconnected, this, &QVNCViewer::disconnectFromVncServer);
    connect(server, SIGNAL(readyRead()), this, SLOT(readServer()));
}

QVNCViewer::~QVNCViewer()
{
    disconnectFromVncServer();
    server->deleteLater();
}

bool QVNCViewer::connectToVncServer(QString ip, quint16 port)
{
    qDebug() << "Trying connect to " << ip << port;
    server->connectToHost(QHostAddress(ip), port);
    if(server->waitForConnected(5000)){
        qDebug() << "Connected to Server\n";
        m_state = Protocol;
        return true;
    }
    else {
        qDebug() << "Not connected to " << ip;
        qDebug() << server->errorString();
        m_state = Disconnected;
        return false;
    }
}

void QVNCViewer::disconnectFromVncServer()
{
    if(m_state == Disconnected)
        return;

    m_state = Disconnected;
    server->close();
}

void QVNCViewer::startFrameBufferUpdate()
{
    if(m_state == Paused || m_state == Connected){
        connect(this, SIGNAL(frameBufferUpdated()), this, SLOT(sendFrameBufferUpdateRequest()));
        sendFrameBufferUpdateRequest();
        m_state = Connected;
    }
}

void QVNCViewer::stopFrameBufferUpdate()
{
    if(m_state != Connected)
        return;
    disconnect(this, SIGNAL(frameBufferUpdated()), this, SLOT(sendFrameBufferUpdateRequest()));
    m_state = Paused;
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

void QVNCViewer::keyPressEvent(QKeyEvent *event)
{
    if(m_state != Connected)
        return;

    QByteArray message(8, 0);

    message[0] = 4;              // keyboard event
    message[1] = 1;              // down = 1 (press)
    message[2] = message[3] = 0; // padding

    qint32 key = event->key();

    message[4] = (key >> 24) & 0xFF;
    message[5] = (key >> 16) & 0xFF;
    message[6] = (key >> 8) & 0xFF;
    message[7] = (key >> 0) & 0xFF;

    server->write(message);
}

void QVNCViewer::keyReleaseEvent(QKeyEvent *event)
{
    if(m_state != Connected)
        return;

    QByteArray message(8, 0);

    message[0] = 4;               // keyboard event
    message[1] = 0;               // down = 0 (release)
    message[2] = message[3] = 0;  // padding

    quint32 key = event->key();

    message[4] = (key >> 24) & 0xFF;
    message[5] = (key >> 16) & 0xFF;
    message[6] = (key >> 8) & 0xFF;
    message[7] = (key >> 0) & 0xFF;

    server->write(message);
}

void QVNCViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(m_state != Connected)
        return;

    QByteArray message(6, 0);
    message[0] = 5;             // mouse event

    switch(event->button())
    {
    case Qt::LeftButton:
        message[1] = 1;
        break;

    case Qt::MiddleButton:
        message[1] = 2;
        break;

    case Qt::RightButton:
        message[1] = 4;
        break;

    default:
        message[1] = 0;
        break;

    }

    quint16 posX = (double(event->pos().x()) / double(width())) * double(frameBufferWidth);
    quint16 posY = (double(event->pos().y()) / double(height())) * double(frameBufferHeight);

    message[2] = (posX >> 8) & 0xFF;
    message[3] = (posX >> 0) & 0xFF;

    message[4] = (posY >> 8) & 0xFF;
    message[5] = (posY >> 0) & 0xFF;

    server->write(message);
}

void QVNCViewer::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    this->mouseMoveEvent(event);
}

void QVNCViewer::mouseReleaseEvent(QMouseEvent *event)
{
    setFocus();
    this->mouseMoveEvent(event);
}

void QVNCViewer::handleFrameBufferUpdate()
{
    QByteArray response;
    if(server->bytesAvailable() >= 3){
        response = server->read(1);      // padding
        response = server->read(2);      // number of rectangles
        quint16 noOfRects = qMakeU16(response.at(0), response.at(1));

        QImage img;
        for(int i=0; i<noOfRects && server->state() == QTcpSocket::ConnectedState; i++)
        {
            response = server->read(4);
            quint32 length = *(quint32*)response.data();

            response = server->read(4);
            int encodingType = qMakeU32(response.at(0), response.at(1), response.at(2), response.at(3));
            if(encodingType == 0)
            {
                QByteArray pixelsData;
                do
                {
                    qApp->processEvents();
                    QByteArray temp = server->read(length);
                    pixelsData.append(temp);
                    length -= temp.size();
                }
                while(length > 0 && server->state() == QTcpSocket::ConnectedState);

                QBuffer qbuff(&pixelsData);
                QImageReader qimg;
                qimg.setDecideFormatFromContent(true);
                qimg.setDevice(&qbuff);
                img = qimg.read();
            }
            screen = std::move(img);
            repaint();
        }
        m_handleMsg = false;
        emit frameBufferUpdated();
    }
}

void QVNCViewer::readServer()
{
    QRfbServerInit response;
    qDebug() << "readServer" << m_state;
    switch (m_state) {
    case Disconnected:
        break;
    case Protocol:
        if(server->bytesAvailable() >= 12){
            char proto[13];
            server->read(proto, 12);
            proto[12] = '\0';

            qDebug("Server protocol version %s", proto);
            server->write("RFB 003.007\n");
            m_state = Authentication;
        }
        break;
    case Authentication:
        if(server->bytesAvailable() >= 1){
            quint32 auth;
            server->read((char *)&auth, sizeof(auth));

            quint8 shared = 0;
            server->write((char *)&shared, sizeof(shared));

            m_state = Init;
        }
        break;
    case Init:
        if(response.read(server)){
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
            m_state = Connected;
            this->startFrameBufferUpdate();
        }
        break;
    case Connected:
        do {
            if (!m_handleMsg) {
                server->read((char *)&m_msgType, 1);
                m_handleMsg = true;
            }
            if (m_handleMsg) {
                switch (m_msgType ) {
                case FramebufferUpdate:
                    handleFrameBufferUpdate();
                    break;
                default:
                    qWarning("Unknown message type: %d", (int)m_msgType);
                    m_handleMsg = false;
                }
            }
        } while (!m_handleMsg && server->bytesAvailable());
        break;
    default:
        break;
    }
}

void QVNCViewer::sendFrameBufferUpdateRequest()
{
    qDebug() << "Sending Frame buffer Update Request\n";
    QByteArray frameBufferUpdateRequest(10, 0);
    frameBufferUpdateRequest[0] = 3;           // message type must be 3
    frameBufferUpdateRequest[1] = 1;           // incremental mode is zero for now (can help optimize the VNC client)
    frameBufferUpdateRequest[2] = 0;           // x position
    frameBufferUpdateRequest[3] = 0;           // x position
    frameBufferUpdateRequest[4] = 0;           // y position
    frameBufferUpdateRequest[5] = 0;           // y position

    frameBufferUpdateRequest[6] = (frameBufferWidth >> 8) & 0xFF;       // width
    frameBufferUpdateRequest[7] = (frameBufferWidth >> 0) & 0xFF;       // width
    frameBufferUpdateRequest[8] = (frameBufferHeight >> 8) & 0xFF;      // height
    frameBufferUpdateRequest[9] = (frameBufferHeight >> 0) & 0xFF;      // height

    server->write(frameBufferUpdateRequest);
}

