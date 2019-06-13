#ifndef MACAPI_H
#define MACAPI_H
#include <QTypeInfo>
#include <ApplicationServices/ApplicationServices.h>
class MacApi{
    static void PostMouseEvent(CGMouseButton button, CGEventType type, const CGPoint point);
public:
    static void PressKey(quint16 unicode);
    static void ReleaseKey(quint16 unicode);
    static void PressMouseLeft(quint16 x, quint16 y);
    static void PressMouseRight(quint16 x, quint16 y);
    static void ReleaseMouseLeft(quint16 x, quint16 y);
    static void ReleaseMouseRight(quint16 x, quint16 y);
    static void MoveMouse(quint16 x, quint16 y);
};

#endif // MACAPI_H
