#include "logindialog.h"
#include "ui_logindialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("系统登录");

    // 设置固定大小，不让用户乱拖
    this->setFixedSize(400, 300);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_btnLogin_clicked()
{
    QString user = ui->editUser->text().trimmed();
    QString pass = ui->editPass->text().trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }

    // 查询数据库对比
    QSqlQuery q;
    q.prepare("SELECT password FROM users WHERE username = ?");
    q.addBindValue(user);

    if (q.exec()) {
        if (q.next()) {
            QString dbPass = q.value(0).toString();
            if (dbPass == pass) {
                // 登录成功
                m_username = user; // 记录名字
                this->accept();    // 关闭窗口并返回 Accepted
            } else {
                QMessageBox::warning(this, "错误", "密码错误！");
            }
        } else {
            QMessageBox::warning(this, "错误", "用户不存在！");
        }
    } else {
        QMessageBox::critical(this, "错误", "数据库查询失败: " + q.lastError().text());
    }
}

void LoginDialog::on_btnExit_clicked()
{
    this->reject(); // 退出程序
}
