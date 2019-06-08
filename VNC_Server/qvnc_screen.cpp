#include "qvnc_screen.h"

QVNCDirtyMap::QVNCDirtyMap(QScreen *screen):
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
