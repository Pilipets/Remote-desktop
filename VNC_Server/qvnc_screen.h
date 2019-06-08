#ifndef QVNC_SCREEN_H
#define QVNC_SCREEN_H
#include <QObject>
#include <QScreen>
class QVNCServer;
class QVNCDirtyMap;

class QVNCScreenPrivate;

class QVNCScreen : public QScreen{
public:
    QVNCScreen(QRect screenSize, int screenId);

    int linestep() const { return image() ? image()->bytesPerLine() : 0; }
    uchar *base() const { return image() ? image()->bits() : 0; }
    QVNCDirtyMap *dirtyMap();
public:
    QVNCScreenPrivate *d_ptr;

private:
    QVNCServer *server;
    QRegion doRedraw();
};

#endif // QVNC_SCREEN_H
