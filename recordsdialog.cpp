#include "recordsdialog.h"
#include "ui_recordsdialog.h"
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QHeaderView>

RecordsDialog::RecordsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordsDialog)
{
    ui->setupUi(this);

    // 1. 初始化关系型模型
    model = new QSqlRelationalTableModel(this);
    model->setTable("stock_record");

    // 2. 设置外键关系 (最重要的一步！)
    // 参数含义:
    // 第1列("goods_id") -> 关联到 "goods" 表 -> 关联字段 "goods_id" -> 显示字段 "goods_name"
    // 注意：字段索引通常从 0 开始，goods_id 是第2列(索引1)
    int goodsIdColumnIndex = model->fieldIndex("goods_id");

    model->setRelation(goodsIdColumnIndex, QSqlRelation("goods", "goods_id", "goods_name"));

    // 3. 查询数据
    model->select();

    // 4. 设置友好的表头名称
    model->setHeaderData(model->fieldIndex("record_id"), Qt::Horizontal, "记录ID");
    model->setHeaderData(goodsIdColumnIndex, Qt::Horizontal, "货品名称"); // 此时显示的是名称了
    model->setHeaderData(model->fieldIndex("record_type"), Qt::Horizontal, "类型");
    model->setHeaderData(model->fieldIndex("record_quantity"), Qt::Horizontal, "数量");
    model->setHeaderData(model->fieldIndex("record_time"), Qt::Horizontal, "时间");
    model->setHeaderData(model->fieldIndex("operator"), Qt::Horizontal, "操作员");

    // 5. 配置 View
    ui->tableView->setModel(model);

    // 这一行是为了让显示更正常（处理关系数据的显示代理）
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    ui->tableView->hideColumn(0); // 隐藏记录ID，用户不需要看
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setColumnWidth(goodsIdColumnIndex, 150); // 拉宽货品名称列
    ui->tableView->setColumnWidth(model->fieldIndex("record_time"), 160); // 拉宽时间列
}

RecordsDialog::~RecordsDialog()
{
    delete ui;
}
