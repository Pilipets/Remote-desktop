#ifndef QVNC_CONNECTION_H
#define QVNC_CONNECTION_H

#include <QtNetwork/QTcpSocket>


quint16 qMakeU16(quint8 l, quint8 h);

quint32 qMakeU32(quint16 l, quint16 h);

quint32 qMakeU32(quint8 lowest, quint8 low, quint8 high, quint8 highest);

class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect(quint16 _x, quint16 _y, quint16 _w, quint16 _h) {
        x = _x; y = _y; w = _w; h = _h;
    }

    void read(QTcpSocket *s);

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
    QRfbServerInit(): name(nullptr),len(0) {}
    ~QRfbServerInit() { delete[] name; }

    bool read(QTcpSocket *s);

    quint16 width;
    quint16 height;
    QRfbPixelFormat format;
    char *name;
    quint32 len;
};


#endif // QVNC_CONNECTION_H
