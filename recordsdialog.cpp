#include "recordsdialog.h"
#include "ui_recordsdialog.h"
#include <QSqlRelation>
#include <QSqlRelationalDelegate>

// === 构造函数 ===
RecordsDialog::RecordsDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::RecordsDialog)
{
    ui->setupUi(this);

    model = new QSqlRelationalTableModel(this);
    model->setTable("stock_record");

    // 关联货品名称
    int goodsIdx = model->fieldIndex("goods_id");
    model->setRelation(goodsIdx, QSqlRelation("goods", "goods_id", "goods_name"));

    // 关联仓库名称
    int wIdx = model->fieldIndex("w_id");
    model->setRelation(wIdx, QSqlRelation("warehouses", "w_id", "w_name"));

    model->select();

    model->setHeaderData(goodsIdx, Qt::Horizontal, "货品名称");
    model->setHeaderData(wIdx, Qt::Horizontal, "所属仓库");
    model->setHeaderData(model->fieldIndex("record_type"), Qt::Horizontal, "类型");
    model->setHeaderData(model->fieldIndex("single_price"), Qt::Horizontal, "单价");
    model->setHeaderData(model->fieldIndex("record_quantity"), Qt::Horizontal, "数量");
    model->setHeaderData(model->fieldIndex("total_price"), Qt::Horizontal, "总价");
    model->setHeaderData(model->fieldIndex("record_time"), Qt::Horizontal, "时间");

    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));
    ui->tableView->hideColumn(0);
    ui->tableView->resizeColumnsToContents();
}

// === 析构函数 (必须要有这个！) ===
RecordsDialog::~RecordsDialog()
{
    delete ui;
}
