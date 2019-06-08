#include "qvnc_client.h"

#include<QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtEndian>
#include <QScreen>
#include <QCoreApplication>
#include <QDebug>
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
    qDebug() << "readClient" << m_state;
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
    qDebug() << "FramebufferUpdateRequest";
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

