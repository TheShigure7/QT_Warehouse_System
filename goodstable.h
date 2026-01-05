#ifndef GOODSTABLE_H
#define GOODSTABLE_H
#include "ui_goodstable.h"

#include <QDialog>
#include <QSqlQuery>

namespace Ui {
class GoodsTable;
}

class GoodsTable : public QDialog
{
    Q_OBJECT

public:
    explicit GoodsTable(QWidget *parent = nullptr);
    ~GoodsTable();


    void setEditData(int goodsId);

private slots:
    // 重写 accept 函数，点击“确定”按钮时触发
    void accept() override;

private:
    Ui::GoodsTable *ui;

    // 初始化仓库下拉框
    void initWarehouseCombo();

    int m_goodsId = -1;
};

#endif // GOODSTABLE_H
