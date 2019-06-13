#include <ApplicationServices/ApplicationServices.h>
#include "MacAPI.h"

void MacApi::PostMouseEvent(CGMouseButton button, CGEventType type, const CGPoint point)
{
     CGEventRef theEvent = CGEventCreateMouseEvent(NULL, type, point, button);
     CGEventSetType(theEvent, type);
     CGEventPost(kCGHIDEventTap, theEvent);
     CFRelease(theEvent);
}

void MacApi::PressKey(quint16 unicode)
{
    CGEventRef downEvt = CGEventCreateKeyboardEvent(NULL, 0, true);
    UniChar oneChar = unicode;
    CGEventKeyboardSetUnicodeString(downEvt,1,&oneChar);
    CGEventPost(kCGAnnotatedSessionEventTap,downEvt);
}


void MacApi::ReleaseKey(quint16 unicode)
{
    CGEventRef upEvt = CGEventCreateKeyboardEvent(NULL, 0, false);
    UniChar oneChar = unicode;
    CGEventKeyboardSetUnicodeString(upEvt,1,&oneChar);
    CGEventPost(kCGAnnotatedSessionEventTap,upEvt);
}

void MacApi::PressMouseLeft(quint16 x, quint16 y)
{
    CGPoint point;
    point.x = x, point.y = y;
    PostMouseEvent(kCGMouseButtonLeft, kCGEventMouseMoved, point);
    PostMouseEvent(kCGMouseButtonLeft, kCGEventLeftMouseDown, point);
}

void MacApi::PressMouseRight(quint16 x, quint16 y)
{
    CGPoint point;
    point.x = x, point.y = y;
    PostMouseEvent(kCGMouseButtonRight, kCGEventMouseMoved, point);
    PostMouseEvent(kCGMouseButtonRight, kCGEventRightMouseDown, point);
}

void MacApi::ReleaseMouseLeft(quint16 x, quint16 y)
{
    CGPoint point;
    point.x = x, point.y = y;
    //PostMouseEvent(kCGMouseButtonLeft, kCGEventMouseMoved, point);
    PostMouseEvent(kCGMouseButtonLeft, kCGEventLeftMouseUp, point);
}

void MacApi::ReleaseMouseRight(quint16 x, quint16 y)
{
    CGPoint point;
    point.x = x, point.y = y;
    //PostMouseEvent(kCGMouseButtonRight, kCGEventMouseMoved, point);
    PostMouseEvent(kCGMouseButtonRight, kCGEventRightMouseUp, point);
}

void MacApi::MoveMouse(quint16 x, quint16 y)
{
    CGPoint point;
    point.x = x, point.y = y;
    PostMouseEvent(kCGMouseButtonCenter, kCGEventMouseMoved, point);
}
