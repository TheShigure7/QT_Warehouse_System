#include "mainwindow.h"
#include "dbmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 在显示窗口前先初始化数据库
    if (!DbManager::initDatabase()) {
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
