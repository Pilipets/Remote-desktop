#include "qvnc_connection.h"
#include <QtEndian>
void QRfbPixelFormat::read(QTcpSocket *s)
{
    char buf[16];
    s->read(buf, 16);
    bitsPerPixel = buf[0];
    depth = buf[1];
    bigEndian = buf[2];
    trueColor = buf[3];

    quint16 a = qFromBigEndian(*(quint16 *)(buf + 4));
    redBits = 0;
    while (a) { a >>= 1; redBits++; }

    a = qFromBigEndian(*(quint16 *)(buf + 6));
    greenBits = 0;
    while (a) { a >>= 1; greenBits++; }

    a = qFromBigEndian(*(quint16 *)(buf + 8));
    blueBits = 0;
    while (a) { a >>= 1; blueBits++; }

    redShift = buf[10];
    greenShift = buf[11];
    blueShift = buf[12];
}

void QRfbPixelFormat::write(QTcpSocket *s)
{
    char buf[16];
    buf[0] = bitsPerPixel;
    buf[1] = depth;
    buf[2] = bigEndian;
    buf[3] = trueColor;

    quint16 a = 0;
    for (int i = 0; i < redBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 4) = qToBigEndian(a);

    a = 0;
    for (int i = 0; i < greenBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 6) = qToBigEndian(a);

    a = 0;
    for (int i = 0; i < blueBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 8) = qToBigEndian(a);

    buf[10] = redShift;
    buf[11] = greenShift;
    buf[12] = blueShift;
    s->write(buf, 16);
}

bool QRfbServerInit::read(QTcpSocket *s)
{
    do{
        if(len == 0 && s->bytesAvailable() >= 8){
            s->read((char *)&width, 2);
            width = qFromBigEndian(width);
            s->read((char *)&height, 2);
            height = qFromBigEndian(height);
            format.read(s);

            s->read((char *)&len, 4);
            len = qFromBigEndian(len);
        }
        else if(len > 0 && s->bytesAvailable() >= len){
            name = new char [len + 1];
            s->read(name, len);
            name[len] = '\0';
            return true;
        }
    }while(s->bytesAvailable() > 0);
    return false;
}

quint16 qMakeU16(quint8 l, quint8 h)
{
    quint16 result;
    quint8 *result_arr = (quint8*)&result;
    result_arr[0] = h;
    result_arr[1] = l;
    return result;
}

quint32 qMakeU32(quint16 l, quint16 h)
{
    quint32 result;
    quint16 *result_arr = (quint16*)&result;
    result_arr[0] = h;
    result_arr[1] = l;
    return result;
}

quint32 qMakeU32(quint8 lowest, quint8 low, quint8 high, quint8 highest)
{
    quint32 result;
    quint8 *result_arr = (quint8*)&result;
    result_arr[0] = highest;
    result_arr[1] = high;
    result_arr[2] = low;
    result_arr[3] = lowest;
    return result;
}

void QRfbRect::read(QTcpSocket *s)
{
    quint16 buf[4];
    s->read((char*)buf, 8);
    x = qFromBigEndian(buf[0]);
    y = qFromBigEndian(buf[1]);
    w = qFromBigEndian(buf[2]);
    h = qFromBigEndian(buf[3]);
}
