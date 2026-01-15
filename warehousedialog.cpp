#include "warehousedialog.h"
#include "ui_warehousedialog.h"

WarehouseDialog::WarehouseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WarehouseDialog)
{
    ui->setupUi(this);
}

WarehouseDialog::~WarehouseDialog()
{
    delete ui;
}
