#include "My_IDE.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    My_IDE w;
    w.resize(800,600);
    w.show();
    return a.exec();
}
