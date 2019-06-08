#ifndef QVNC_P_H
#define QVNC_P_H

#include<QObject>
#include<QVector>
class QScreen;
class QTcpServer;
class QVncClient;

class QVncServer : public QObject
{
    Q_OBJECT
public:
    QVncServer(QScreen *screen, quint16 port = 5900);
    ~QVncServer();

    enum ServerMsg { FramebufferUpdate = 0};

    void setDirty();


    inline QScreen* screen() const { return qvnc_screen; }
    QImage screenImage() const;
    void discardClient(QVncClient *client);

private slots:
    void newConnection();
    void init();

private:
    QTcpServer *serverSocket;
    QVector<QVncClient*> clients;
    QScreen *qvnc_screen;
    quint16 m_port;
};
#endif // QVNC_P_H
