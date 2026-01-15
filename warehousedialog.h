#ifndef WAREHOUSEDIALOG_H
#define WAREHOUSEDIALOG_H

#include <QDialog>

namespace Ui {
class WarehouseDialog;
}

class WarehouseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarehouseDialog(QWidget *parent = nullptr);
    ~WarehouseDialog();

    // 【核心】加载数据（传入 -1 代表新增，传入具体ID代表修改）
    void loadData(int w_id);

private slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:
    Ui::WarehouseDialog *ui;
    int m_id = -1; // 记录当前操作的仓库ID
};

#endif // WAREHOUSEDIALOG_H
