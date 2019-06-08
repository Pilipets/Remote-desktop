#ifndef QVNC_SCREEN_H
#define QVNC_SCREEN_H
#include<QObject>
#include<QScreen>
#define MAP_TILE_SIZE 16
#define MAP_WIDTH 1280 / MAP_TILE_SIZE
#define MAP_HEIGHT 1024 / MAP_TILE_SIZE
#include "qvncserver.h"
class QVNCScreen;
class QVNCDirtyMap
{
public:
    QVNCDirtyMap(QVNCScreen *screen);
    virtual ~QVNCDirtyMap();

    void reset();
    bool dirty(int x, int y) const;
    virtual void setDirty(int x, int y, bool force = false) = 0;
    void setClean(int x, int y);

    int bytesPerPixel;

    int numDirty;
    int mapWidth;
    int mapHeight;

protected:
    uchar *map;
    QVNCScreen *screen;
    uchar *buffer;
    int bufferWidth;
    int bufferHeight;
    int bufferStride;
    int numTiles;
};

template <class T>
class QVNCDirtyMapOptimized : public QVNCDirtyMap
{
public:
    QVNCDirtyMapOptimized(QVNCScreen *screen) : QVNCDirtyMap(screen) {}
    ~QVNCDirtyMapOptimized() {}

    void setDirty(int x, int y, bool force = false);
};

class QVNCScreenPrivate : public QObject
{
public:
    QVNCScreenPrivate(QVNCScreen *parent, int screenId);
    ~QVNCScreenPrivate();

    void setDirty(const QRect &rect, bool force = false);
    //void configure();

    qreal dpiX;
    qreal dpiY;
    bool doOnScreenSurface;
    QVNCDirtyMap *dirty;
    int refreshRate;
    QVNCServer *vncServer;
    QVNCScreen *q_ptr;
};

#endif // QVNC_SCREEN_H
