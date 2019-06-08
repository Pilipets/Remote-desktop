#ifndef QVNC_CLIENT_H
#define QVNC_CLIENT_H

#include "qvnc_server.h"
#include "qvnc_connection.h"
#include <QObject>
#include <QRegion>

class QTcpSocket;
class QVncServer;

class QVncClient : public QObject
{
    Q_OBJECT
public:
    enum ClientMsg {
        SetPixelFormat = 0,
        FramebufferUpdateRequest = 3,
    };

    explicit QVncClient(QTcpSocket *clientSocket, QVncServer *server);
    ~QVncClient();
    QTcpSocket *clientSocket() const;
    QVncServer *server() const { return m_server; }

    inline bool isConnected() const { return m_state == Connected; }

    inline int clientBytesPerPixel() const {
        return m_pixelFormat.bitsPerPixel / 8;
    }

private slots:
    void readClient();
    void discardClient();
    void checkUpdate();

private:
    enum ClientState {
        Disconnected,
        Protocol,
        Authentication,
        Init,
        Connected
    };
    enum ProtocolVersion {
        V3_7,
        V3_8
    };

    void frameBufferUpdateRequest();

    QVncServer *m_server;
    QTcpSocket *m_clientSocket;
    QRfbEncoder *m_encoder;

    // Client State
    ClientState m_state;
    quint8 m_msgType;
    bool m_handleMsg;
    QRfbPixelFormat m_pixelFormat;

    bool m_wantUpdate;

    ProtocolVersion m_protocolVersion;
};

#endif // QVNC_CLIENT_H
