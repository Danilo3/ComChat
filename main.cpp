#include "mainchatwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainChatWindow w;
    w.show();
    return a.exec();
}
