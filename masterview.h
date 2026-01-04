#ifndef MASTERVIEW_H
#define MASTERVIEW_H

#include <QWidget>
#include <QButtonGroup>

// 引入你之前写的两个页面类
#include "mainwindow.h"
#include "recordsdialog.h"

namespace Ui {
class MasterView;
}

class MasterView : public QWidget
{
    Q_OBJECT

public:
    explicit MasterView(QWidget *parent = nullptr);
    ~MasterView();

private slots:
    // 按钮点击槽函数
    void on_btnPageGoods_clicked();
    void on_btnPageRecords_clicked();

private:
    Ui::MasterView *ui;

    // 页面指针
    MainWindow *pageGoods;
    RecordsDialog *pageRecords;

    // 辅助函数：初始化界面
    void initViews();
};

#endif // MASTERVIEW_H
