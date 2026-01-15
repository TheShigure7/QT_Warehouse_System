#include "warehousepage.h"
#include "ui_warehousepage.h"

WarehousePage::WarehousePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WarehousePage)
{
    ui->setupUi(this);
}

WarehousePage::~WarehousePage()
{
    delete ui;
}
