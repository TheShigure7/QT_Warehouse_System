#ifndef WAREHOUSEPAGE_H
#define WAREHOUSEPAGE_H

#include <QWidget>

namespace Ui {
class WarehousePage;
}

class WarehousePage : public QWidget
{
    Q_OBJECT

public:
    explicit WarehousePage(QWidget *parent = nullptr);
    ~WarehousePage();

private:
    Ui::WarehousePage *ui;
};

#endif // WAREHOUSEPAGE_H
