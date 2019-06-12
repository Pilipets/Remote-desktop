#ifndef MACAPI_H
#define MACAPI_H
#include <QTypeInfo>

class MacApi{
public:
    static void PressKey(quint16 unicode);
    static void ReleaseKey(quint16 unicode);
};

#endif // MACAPI_H
