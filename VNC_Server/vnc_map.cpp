#include "qvnc_screen.h"

QVNCDirtyMap::QVNCDirtyMap(QVNCScreen *screen):
    bytesPerPixel(0), numDirty(0), screen(screen)
{
    bytesPerPixel = (screen->depth() + 7) / 8;
    QSize screenSize = screen->geometry().size();
    bufferWidth = screenSize.width();
    bufferHeight = screenSize.height();
    bufferStride = bufferWidth * bytesPerPixel;
    buffer = new uchar[bufferHeight * bufferStride];

    mapWidth = (bufferWidth + MAP_TILE_SIZE - 1) / MAP_TILE_SIZE;
    mapHeight = (bufferHeight + MAP_TILE_SIZE - 1) / MAP_TILE_SIZE;
    numTiles = mapWidth * mapHeight;
    map = new uchar[numTiles];
}

QVNCDirtyMap::~QVNCDirtyMap()
{
    delete[] map;
    delete[] buffer;
}

//mark all dirty map as modified
void QVNCDirtyMap::reset()
{
    memset(map, 1, numTiles);
    memset(buffer, 0, bufferHeight * bufferStride);
    numDirty = numTiles;
}

inline bool QVNCDirtyMap::dirty(int x, int y) const
{
    return map[y * mapWidth + x];
}

//set certain pixel as not modified - as sent to the client
void QVNCDirtyMap::setClean(int x, int y)
{
    map[y * mapWidth + x] = 0;
    --numDirty;
}

template<class T>
void QVNCDirtyMapOptimized<T>::setDirty(int tileX, int tileY, bool force)
{
    bool changed = false;

    if (!force) {
        const int lstep = screen->linestep();
        const int startX = tileX * MAP_TILE_SIZE;
        const int startY = tileY * MAP_TILE_SIZE;
        const uchar *scrn = screen->base() + startY * lstep + startX * bytesPerPixel;
        uchar *old = buffer + startY * bufferStride + startX * sizeof(T);

        const int tileHeight = (startY + MAP_TILE_SIZE > bufferHeight ?
                                    bufferHeight - startY : MAP_TILE_SIZE);
        const int tileWidth = (startX + MAP_TILE_SIZE > bufferWidth ?
                                   bufferWidth - startX : MAP_TILE_SIZE);
        const bool doInlines = (tileWidth == MAP_TILE_SIZE);

        int y = tileHeight;

        if (doInlines) { // hw: memcmp/memcpy is inlined when using constants
            while (y) {
                if (memcmp(old, scrn, sizeof(T) * MAP_TILE_SIZE)) {
                    changed = true;
                    break;
                }
                scrn += lstep;

                old += bufferStride;
                --y;
            }

            while (y) {
                memcpy(old, scrn, sizeof(T) * MAP_TILE_SIZE);
                scrn += lstep;
                old += bufferStride;
                --y;
            }
        } else {
            while (y) {
                if (memcmp(old, scrn, sizeof(T) * tileWidth)) {
                    changed = true;
                    break;
                }
                scrn += lstep;
                old += bufferStride;
                --y;
            }

            while (y) {
                memcpy(old, scrn, sizeof(T) * tileWidth);
                scrn += lstep;
                old += bufferStride;
                --y;
            }
        }
    }

    const int mapIndex = tileY * mapWidth + tileX;
    if ((force || changed) && !map[mapIndex]) {
        map[mapIndex] = 1;
        ++numDirty;
    }
}

QVNCScreenPrivate::QVNCScreenPrivate(QVNCScreen *parent, int screenId):
    dpiX(72), dpiY(72), doOnScreenSurface(false), refreshRate(25),
          vncServer(0), q_ptr(parent)
{
    vncServer = new QVNCServer(q_ptr);
    vncServer->setRefreshRate(refreshRate);


    Q_ASSERT(q_ptr->depth() == 32);
    dirty = new QVNCDirtyMapOptimized<quint32>(q_ptr);
}

QVNCScreenPrivate::~QVNCScreenPrivate()
{
    delete vncServer;
    delete dirty;
}

void QVNCScreenPrivate::setDirty(const QRect &rect, bool force)
{
    if (rect.isEmpty())
            return;
    //     if (q_ptr->screen())
    //         q_ptr->screen()->setDirty(rect);

    if (!vncServer || !vncServer->isConnected()) {
    //        qDebug() << "QVNCScreenPrivate::setDirty() - Not connected";
        return;
    }

    const QRect r = rect; // .translated(-q_ptr->offset());
    const int x1 = r.x() / MAP_TILE_SIZE;
    int y = r.y() / MAP_TILE_SIZE;
    for (; (y <= r.bottom() / MAP_TILE_SIZE) && y < dirty->mapHeight; y++)
        for (int x = x1; (x <= r.right() / MAP_TILE_SIZE) && x < dirty->mapWidth; x++)
            dirty->setDirty(x, y, force);

    vncServer->setDirty();
}
