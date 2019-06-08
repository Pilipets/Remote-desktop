#ifndef QVNC_CONNECTION_H
#define QVNC_CONNECTION_H

#include<QObject>
#include<QByteArray>
class QTcpSocket;
class QVncClient;
class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect(quint16 _x, quint16 _y, quint16 _w, quint16 _h) {
        x = _x; y = _y; w = _w; h = _h;
    }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s) const;

    quint16 x;
    quint16 y;
    quint16 w;
    quint16 h;
};
class QRfbPixelFormat
{
public:
    static int size() { return 16; }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint8 bitsPerPixel;
    quint8 depth;
    bool bigEndian;
    bool trueColor;
    quint16 redBits;
    quint16 greenBits;
    quint16 blueBits;
    quint8 redShift;
    quint8 greenShift;
    quint8 blueShift;
};

class QRfbServerInit
{
public:
    QRfbServerInit() { name = nullptr; }
    ~QRfbServerInit() { delete[] name; }

    int size() const { return QRfbPixelFormat::size() + 8 + strlen(name); }
    void setName(const char *n);

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint16 width;
    quint16 height;
    QRfbPixelFormat format;
    char *name;
};

class QRfbFrameBufferUpdateRequest
{
public:
    bool read(QTcpSocket *s);

    char incremental;
    QRfbRect rect;
};

class QRfbEncoder
{
public:
    QRfbEncoder(QVncClient *s) : client(s) {}
    virtual ~QRfbEncoder() {}

    virtual void write() = 0;

protected:
    QVncClient *client;
};

class QRfbRawEncoder : public QRfbEncoder
{
public:
    QRfbRawEncoder(QVncClient *s) : QRfbEncoder(s) {}

    void write() override;

private:
    QByteArray buffer;
};

#endif // QVNC_CONNECTION_H
