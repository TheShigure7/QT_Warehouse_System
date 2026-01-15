#include "mainwindow.h"
#include "dbmanager.h"
#include "masterview.h"
#include "logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 初始化数据库
    if (!DbManager::initDatabase()) {
        return -1;
    }

    // 2. 显示登录窗口
    LoginDialog loginDlg;
    if (loginDlg.exec() == QDialog::Accepted) {
        // 3. 只有登录成功了，才创建并显示主界面
        MasterView w;

        // 把登录的名字传进去
        w.initUser(loginDlg.getUsername());

        w.show();
        return a.exec();
    } else {
        // 如果点击了取消或关闭登录框，直接退出程序
        return 0;
    }
}
