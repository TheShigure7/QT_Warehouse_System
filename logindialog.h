#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    //获取当前登录成功的用户名
    QString getUsername() const { return m_username; }

private slots:
    void on_btnLogin_clicked();
    void on_btnExit_clicked();

private:
    Ui::LoginDialog *ui;
    QString m_username; // 内部存储成功的用户名
};

#endif // LOGINDIALOG_H
