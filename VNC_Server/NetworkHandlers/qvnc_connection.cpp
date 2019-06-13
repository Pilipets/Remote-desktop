#include "qvnc_connection.h"
#include "VNCServer/qvnc_client.h"
#include <QtEndian>
#include <QtNetwork/QTcpSocket>
#include <QScreen>
#include <QPixmap>
#include <QDebug>
#include <QApplication>
#include <QKeySequence>
#include <QString>
#include <QBuffer>

static const struct {
    int keysym;
    int keycode;
} keyMap[] = {
    { 0xff08, Qt::Key_Backspace }, { 0xff09, Qt::Key_Tab       }, { 0xff0d, Qt::Key_Return    },
    { 0xff1b, Qt::Key_Escape    }, { 0xff63, Qt::Key_Insert    }, { 0xffff, Qt::Key_Delete    },
    { 0xff50, Qt::Key_Home      }, { 0xff57, Qt::Key_End       }, { 0xff55, Qt::Key_PageUp    },
    { 0xff56, Qt::Key_PageDown  }, { 0xff51, Qt::Key_Left      }, { 0xff52, Qt::Key_Up        },
    { 0xff53, Qt::Key_Right     }, { 0xff54, Qt::Key_Down      }, { 0xffbe, Qt::Key_F1        },
    { 0xffbf, Qt::Key_F2        }, { 0xffc0, Qt::Key_F3        }, { 0xffc1, Qt::Key_F4        },
    { 0xffc2, Qt::Key_F5        }, { 0xffc3, Qt::Key_F6        }, { 0xffc4, Qt::Key_F7        },
    { 0xffc5, Qt::Key_F8        }, { 0xffc6, Qt::Key_F9        }, { 0xffc7, Qt::Key_F10       },
    { 0xffc8, Qt::Key_F11       }, { 0xffc9, Qt::Key_F12       }, { 0xffe1, Qt::Key_Shift     },
    { 0xffe2, Qt::Key_Shift     }, { 0xffe3, Qt::Key_Control   }, { 0xffe4, Qt::Key_Control   },
    { 0xffe7, Qt::Key_Meta      }, { 0xffe8, Qt::Key_Meta      }, { 0xffe9, Qt::Key_Alt       },
    { 0xffea, Qt::Key_Alt       }, { 0xffb0, Qt::Key_0         }, { 0xffb1, Qt::Key_1         },
    { 0xffb2, Qt::Key_2         }, { 0xffb3, Qt::Key_3         }, { 0xffb4, Qt::Key_4         },
    { 0xffb5, Qt::Key_5         }, { 0xffb6, Qt::Key_6         }, { 0xffb7, Qt::Key_7         },
    { 0xffb8, Qt::Key_8         }, { 0xffb9, Qt::Key_9         }, { 0xff8d, Qt::Key_Return    },
    { 0xffaa, Qt::Key_Asterisk  }, { 0xffab, Qt::Key_Plus      }, { 0xffad, Qt::Key_Minus     },
    { 0xffae, Qt::Key_Period    }, { 0xffaf, Qt::Key_Slash     }, { 0xff95, Qt::Key_Home      },
    { 0xff96, Qt::Key_Left      }, { 0xff97, Qt::Key_Up        }, { 0xff98, Qt::Key_Right     },
    { 0xff99, Qt::Key_Down      }, { 0xff9a, Qt::Key_PageUp    }, { 0xff9b, Qt::Key_PageDown  },
    { 0xff9c, Qt::Key_End       }, { 0xff9e, Qt::Key_Insert    }, { 0xff9f, Qt::Key_Delete    },
    { 0, 0 }
};

bool QRfbKeyEvent::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 7)
        return false;

    s->read(&down, 1);
    quint16 tmp;
    s->read((char *)&tmp, 2);  // padding
    quint32 key;
    s->read((char *)&key, 4);
    key = qFromBigEndian(key);
    unicode = 0;
    keycode = 0;
    int i = 0;
    while (keyMap[i].keysym && !keycode) {
        if (keyMap[i].keysym == (int)key)
            keycode = keyMap[i].keycode;
        i++;
    }
    if (keycode >= ' ' && keycode <= '~')
        unicode = keycode;
    if (!keycode) {
        if (key <= 0xff) {
            unicode = key;
            if (key >= 'a' && key <= 'z')
                keycode = key;
                //keycode = Qt::Key_A + key - 'a';
            else if (key >= ' ' && key <= '~')
                keycode = Qt::Key_Space + key - ' ';
        }
    }

    if(down & 1)
        qDebug() << QKeySequence(keycode).toString();
    return true;
}

bool QRfbPointerEvent::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 5)
        return false;

    char buttonMask;
    s->read(&buttonMask, 1);
    buttons = Qt::NoButton;
    if (buttonMask & 1)
        buttons |= Qt::LeftButton;
    if (buttonMask & 2)
        buttons |= Qt::MidButton;
    if (buttonMask & 4)
        buttons |= Qt::RightButton;

    quint16 tmp;
    s->read((char *)&tmp, 2);
    x = qFromBigEndian(tmp);
    s->read((char *)&tmp, 2);
    y = qFromBigEndian(tmp);

    return true;
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

void QRfbRect::write(QTcpSocket *s) const
{
    quint16 buf[4];
    buf[0] = qToBigEndian(x);
    buf[1] = qToBigEndian(y);
    buf[2] = qToBigEndian(w);
    buf[3] = qToBigEndian(h);
    s->write((char*)buf, 8);
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

void QRfbServerInit::setName(const char *n)
{
    delete[] name;
    name = new char [strlen(n) + 1];
    strcpy(name, n);
}

void QRfbServerInit::write(QTcpSocket *s)
{
    quint16 t = qToBigEndian(width);
    s->write((char *)&t, 2);
    t = qToBigEndian(height);
    s->write((char *)&t, 2);
    format.write(s);
    quint32 len = strlen(name);
    len = qToBigEndian(len);
    s->write((char *)&len, 4);
    s->write(name, strlen(name));
}


bool QRfbFrameBufferUpdateRequest::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 9)
        return false;

    s->read(&incremental, 1);
    rect.read(s);

    return true;
}

void QRfbRawEncoder::write()
{
    //qDebug() << "QRfbRawEncoder::write()";
    QTcpSocket *socket = client->clientSocket();

    const char tmp[2] = { 0, 0 }; // msg type, padding
    socket->write(tmp, sizeof(tmp));

    const quint16 count = qToBigEndian(quint16(1)); //rectangle amount
    socket->write((char *)&count, sizeof(count));

    QScreen* const screen = client->server()->screen();
    const QSize screenSize = screen->geometry().size();

    const int bytesPerPixel = client->clientBytesPerPixel();
    const QPixmap originalPixMap = screen->grabWindow(0,0,0,screenSize.width(), screenSize.height());
    const QImage screenImage = originalPixMap.toImage();

    QByteArray im;
    QBuffer bufferJpeg(&im);
    bufferJpeg.open(QIODevice::WriteOnly);
    screenImage.save(&bufferJpeg, "JPG");

    qint32 size = bufferJpeg.data().size();
    QByteArray size_array((const char*)&size,4);
    socket->write(size_array);
    //const QRfbRect rect(0,0,screenSize.width(),screenSize.height());
    //rect.write(socket);


    const quint32 encoding = qToBigEndian(0); // raw encoding
    socket->write((char *)&encoding, sizeof(encoding));

    /*int linestep = screenImage.bytesPerLine();
    const uchar *screendata = screenImage.scanLine(rect.y) + rect.x * screenImage.depth() / 8;

    for (int i = 0; i < rect.h && socket->state() == QTcpSocket::ConnectedState; ++i) {
        //qApp->processEvents();
        socket->write((const char*)screendata, rect.w * bytesPerPixel);
        screendata += linestep;
    }*/
    socket->write(bufferJpeg.data());
    //qApp->processEvents();
    socket->flush();

}
