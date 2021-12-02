#include "widget.h"
#include <QApplication>
#include <QDebug>
#include "connection.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (createConnection())
        qDebug()<< "连接数据库ok";
    else
        return 1;

    Widget w;
    w.show();

    return a.exec();
}
