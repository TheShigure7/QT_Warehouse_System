#ifndef WAREHOUSEPAGE_H
#define WAREHOUSEPAGE_H

#include <QWidget>
#include <QSqlTableModel>

namespace Ui {
class WarehousePage;
}

class WarehousePage : public QWidget
{
    Q_OBJECT

public:
    explicit WarehousePage(QWidget *parent = nullptr);
    ~WarehousePage();

    // 提供一个刷新函数，供外部调用（可选）
    void refreshData();

private slots:
    void on_btnWhAdd_clicked();
    void on_btnWhEdit_clicked();
    void on_btnWhDel_clicked();

private:
    Ui::WarehousePage *ui;
    QSqlTableModel *model; // 专门管理 warehouses 表

    void initTable();
};

#endif // WAREHOUSEPAGE_H
