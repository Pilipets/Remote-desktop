#include "ServerWindow/serverstartwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerStartWindow w;
    w.show();

    return a.exec();
}
