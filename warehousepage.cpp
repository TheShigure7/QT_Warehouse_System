#include "warehousepage.h"
#include "ui_warehousepage.h"
#include "warehousedialog.h" // 引入之前的弹窗头文件
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

WarehousePage::WarehousePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WarehousePage)
{
    ui->setupUi(this);
    initTable();
}

WarehousePage::~WarehousePage()
{
    delete ui;
}

void WarehousePage::initTable()
{
    model = new QSqlTableModel(this);
    model->setTable("warehouses");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // 手动提交，防止误操作
    model->select();

    // 设置表头
    model->setHeaderData(model->fieldIndex("w_name"), Qt::Horizontal, "仓库名称");
    model->setHeaderData(model->fieldIndex("w_address"), Qt::Horizontal, "地址");
    model->setHeaderData(model->fieldIndex("w_capacity"), Qt::Horizontal, "容量");
    model->setHeaderData(model->fieldIndex("w_count"), Qt::Horizontal, "库存数");
    model->setHeaderData(model->fieldIndex("w_value"), Qt::Horizontal, "总价值");

    ui->viewWarehouse->setModel(model);
    ui->viewWarehouse->hideColumn(0); // 隐藏ID列
    ui->viewWarehouse->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->viewWarehouse->setAlternatingRowColors(true);
    ui->viewWarehouse->horizontalHeader()->setStretchLastSection(true);

    ui->viewWarehouse->setColumnWidth(1, 200);
    ui->viewWarehouse->setColumnWidth(2, 400);
}

void WarehousePage::refreshData()
{
    model->select();
}

// 1. 新建仓库
void WarehousePage::on_btnWhAdd_clicked()
{
    WarehouseDialog dlg(this);
    dlg.setWindowTitle("新建仓库");

    if (dlg.exec() == QDialog::Accepted) {
        model->select();
        QMessageBox::information(this, "成功", "仓库已添加！");
    }
}

// 2. 修改仓库
void WarehousePage::on_btnWhEdit_clicked()
{
    QModelIndex index = ui->viewWarehouse->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择一个仓库");
        return;
    }

    int row = index.row();
    int w_id = model->record(row).value("w_id").toInt();

    WarehouseDialog dlg(this);
    dlg.loadData(w_id); // 调用弹窗的加载数据

    if (dlg.exec() == QDialog::Accepted) {
        model->select();
        QMessageBox::information(this, "成功", "仓库信息已修改！");
    }
}

// 3. 删除仓库
void WarehousePage::on_btnWhDel_clicked()
{
    QModelIndex index = ui->viewWarehouse->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    QString name = model->record(row).value("w_name").toString();
    int w_id = model->record(row).value("w_id").toInt();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除仓库【%1】吗？\n警告：该仓库下的所有商品也会被一并删除！").arg(name),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM warehouses WHERE w_id = ?");
        q.addBindValue(w_id);

        if (q.exec()) {
            model->select();
            QMessageBox::information(this, "成功", "仓库已删除");
        } else {
            QMessageBox::critical(this, "错误", "删除失败");
        }
    }
}
