#include "qvnc_client.h"

#include<QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtEndian>
#include <QScreen>
#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>
#include "MacEventHandlers/MacAPI.h"
#include <QKeySequence>

QVncClient::QVncClient(QTcpSocket *clientSocket, QVncServer *server):
    QObject (server), m_server(server), m_clientSocket(clientSocket),
    m_encoder(nullptr),m_msgType(0),m_handleMsg(false),
    m_wantUpdate(false), m_protocolVersion(V3_8)
{

    connect(m_clientSocket,SIGNAL(readyRead()),this,SLOT(readClient()));
    connect(m_clientSocket,SIGNAL(disconnected()),this,SLOT(discardClient()));

    // send protocol version
    const char *proto = "RFB 003.007\n";
    m_clientSocket->write(proto, 12);
    m_state = Protocol;

    m_encoder = new QRfbRawEncoder(this);
}

QVncClient::~QVncClient()
{
    //m_clientSocket->disconnect();
    m_clientSocket->disconnect();
    m_clientSocket->deleteLater();
    delete m_encoder;
    qDebug() << "client has removed properly";
}

QTcpSocket *QVncClient::clientSocket() const
{
    return  m_clientSocket;
}

void QVncClient::readClient()
{
    //qDebug() << "readClient" << m_state;
    switch (m_state) {
    case Disconnected:
        break;
    case Protocol:
        if (m_clientSocket->bytesAvailable() >= 12) {
            char proto[13];
            m_clientSocket->read(proto, 12);
            proto[12] = '\0';
            qDebug("Client protocol version %s", proto);
            if (!strcmp(proto, "RFB 003.008\n")) {
                m_protocolVersion = V3_8;
            } else if (!strcmp(proto, "RFB 003.007\n")) {
                m_protocolVersion = V3_7;
            }

            if (m_protocolVersion == V3_7) {
                // No authentication
                quint32 auth = qToBigEndian(1);
                m_clientSocket->write((char *) &auth, sizeof(auth));
                m_state = Init;

            }
        }
        break;
    case Authentication:
        break;
    case Init:
        if (m_clientSocket->bytesAvailable() >= 1) {
            quint8 shared;
            m_clientSocket->read((char *) &shared, 1);

            // Server Init msg

            QRfbServerInit sim;
            QRfbPixelFormat &format = sim.format;
            switch (m_server->screen()->depth()) {
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
            default:
                qWarning("QVNC cannot drive depth %d", m_server->screen()->depth());
                discardClient();
                return;
            }
            sim.width = m_server->screen()->geometry().width();
            sim.height = m_server->screen()->geometry().height();
            sim.setName("Qt for Mac OS VNC Server");
            sim.write(m_clientSocket);
            m_pixelFormat = format;

            m_state = Connected;
        }
        break;

    case Connected:
        do {
            if (!m_handleMsg) {
                m_clientSocket->read((char *)&m_msgType, 1);
                m_handleMsg = true;
            }
            if (m_handleMsg) {
                switch (m_msgType ) {
                case FramebufferUpdateRequest:
                    frameBufferUpdateRequest();
                    break;
                case KeyEvent:
                    keyEvent();
                    break;
                case PointerEvent:
                    pointerEvent();
                    break;
                default:
                    qWarning("Unknown message type: %d", (int)m_msgType);
                    m_handleMsg = false;
                }
            }
        } while (!m_handleMsg && m_clientSocket->bytesAvailable());
        break;
    default:
        break;
    }
}

void QVncClient::discardClient()
{
    m_state = Disconnected;
    m_server->discardClient(this);
}

void QVncClient::checkUpdate()
{
    if (!m_wantUpdate)
            return;

    if (m_encoder)
        m_encoder->write();
    m_wantUpdate = false;
}


void QVncClient::frameBufferUpdateRequest()
{
    //qDebug() << "FramebufferUpdateRequest";
    QRfbFrameBufferUpdateRequest ev;

    if (ev.read(m_clientSocket)) {
        if (!ev.incremental) {
            QRect r(ev.rect.x, ev.rect.y, ev.rect.w, ev.rect.h);
            //r.translate(m_server->screen()->geometry().topLeft());
            //setDirty(r);
        }
        m_wantUpdate = true;
        checkUpdate();
        m_handleMsg = false;
    }
}

void QVncClient::keyEvent()
{
    QRfbKeyEvent ev;

    if (ev.read(m_clientSocket)) {
        if (ev.keycode == Qt::Key_Shift)
            m_keymod = ev.down ? m_keymod | Qt::ShiftModifier :
                                 m_keymod & ~Qt::ShiftModifier;
        else if (ev.keycode == Qt::Key_Control)
            m_keymod = ev.down ? m_keymod | Qt::ControlModifier :
                                 m_keymod & ~Qt::ControlModifier;
        else if (ev.keycode == Qt::Key_Alt)
            m_keymod = ev.down ? m_keymod | Qt::AltModifier :
                                 m_keymod & ~Qt::AltModifier;
        if (ev.unicode || ev.keycode){
            if(ev.down)
                MacApi::PressKey(ev.unicode);
            else
                MacApi::ReleaseKey(ev.unicode);

            QString repr = QKeySequence(ev.keycode).toString();
            if(ev.down)
                qDebug() << "Process button pressed event" << (repr.isEmpty() ? repr : QString::number(ev.keycode));
            else
                qDebug() << "Process button released event" << (repr.isEmpty() ? repr : QString::number(ev.keycode));
            //QWindowSystemInterface::handleKeyEvent(0, ev.down ? QEvent::KeyPress : QEvent::KeyRelease, ev.keycode, m_keymod, QString(ev.unicode));
        }

        m_handleMsg = false;
    }
}

void QVncClient::pointerEvent()
{
    QRfbPointerEvent ev;
    if (ev.read(m_clientSocket)) {
        const QPoint pos = m_server->screen()->geometry().topLeft() + QPoint(ev.x, ev.y);
        //qDebug() << "Mouse moved to " << pos.x() << " " << pos.y();


        if(ev.buttons == Qt::LeftButton)
        {
            leftPressed = !leftPressed;
            qDebug() << "Left button event";
            if(leftPressed)
                MacApi::PressMouseLeft(pos.x(),pos.y());
            else
                MacApi::ReleaseMouseLeft(pos.x(), pos.y());
        }
        else if(ev.buttons == Qt::RightButton){
            rightPressed = !rightPressed;
            qDebug() << "Right button event";
            if(rightPressed)
                MacApi::PressMouseRight(pos.x(), pos.y());
            else
                MacApi::ReleaseMouseRight(pos.x(),pos.y());
        }
        else{
            MacApi::MoveMouse(pos.x(), pos.y());
        }
        //QWindowSystemInterface::handleMouseEvent(0, pos, pos, ev.buttons, QGuiApplication::keyboardModifiers());
        m_handleMsg = false;
    }
}

