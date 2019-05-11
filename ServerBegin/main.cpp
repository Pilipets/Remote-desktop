#include <QCoreApplication>
#include "qvncserver.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //QVNCServer* s = new QVNCServer();

    return a.exec();
}
