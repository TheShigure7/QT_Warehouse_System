#include "recordsdialog.h"
#include "ui_recordsdialog.h"

RecordsDialog::RecordsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecordsDialog)
{
    ui->setupUi(this);
}

RecordsDialog::~RecordsDialog()
{
    delete ui;
}
