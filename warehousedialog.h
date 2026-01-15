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

private:
    Ui::WarehouseDialog *ui;
};

#endif // WAREHOUSEDIALOG_H
