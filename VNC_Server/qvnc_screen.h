#ifndef QVNC_SCREEN_H
#define QVNC_SCREEN_H
#include<QObject>
#include<QScreen>
#define MAP_TILE_SIZE 16
#define MAP_WIDTH 1280 / MAP_TILE_SIZE
#define MAP_HEIGHT 1024 / MAP_TILE_SIZE
class QVNCScreen;
class QVNCDirtyMap
{
public:
    QVNCDirtyMap(QScreen *screen);
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
    QScreen *screen;
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
    QVNCDirtyMapOptimized(QScreen *screen) : QVNCDirtyMap(screen) {}
    ~QVNCDirtyMapOptimized() {}

    void setDirty(int x, int y, bool force = false);
};
#endif // QVNC_SCREEN_H
