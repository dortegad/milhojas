#include <QApplication>
#include "mwmilhojas.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MWMilHojas w;
    w.show();
    
    return a.exec();
}
