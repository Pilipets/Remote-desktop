//#include <ApplicationServices/ApplicationServices.h>
#include "MacAPI.h"

void MacApi::PressKey(quint16 unicode)
{
    //CGEventRef downEvt = CGEventCreateKeyboardEvent(NULL, 0, true);
    //UniChar oneChar = unicode;
    //CGEventKeyboardSetUnicodeString(downEvt,1,&oneChar);
    //CGEventPost(kCGAnnotatedSessionEventTap,downEvt);
}


void MacApi::ReleaseKey(quint16 unicode)
{
    //CGEventRef upEvt = CGEventCreateKeyboardEvent(NULL, 0, false);
    //UniChar oneChar = unicode;
    //CGEventKeyboardSetUnicodeString(upEvt,1,&oneChar);
    //CGEventPost(kCGAnnotatedSessionEventTap,upEvt);
}
