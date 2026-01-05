#include "goodstable.h"
#include "ui_goodstable.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>

GoodsTable::GoodsTable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GoodsTable)
{
    ui->setupUi(this);

    // 1. 初始化仓库下拉框数据
    initWarehouseCombo();

    // 2. 设置一些默认值，方便测试
    ui->priceEdit->setText("0");
    ui->stockQuantityEdit->setText("0");
    ui->warningQuantityEdit->setText("10");
}

GoodsTable::~GoodsTable()
{
    delete ui;
}

void GoodsTable::initWarehouseCombo()
{
    ui->wareBox->clear();

    QSqlQuery query("SELECT w_id, w_name FROM warehouses");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        // 【关键】显示的是名字，存的是ID
        ui->wareBox->addItem(name, id);
    }
}

// 当用户点击 ButtonBox 的 "OK" / "确定" 时会自动调用这个函数
void GoodsTable::accept()
{
    // 1. 获取输入数据
    QString name = ui->nameEdit->text().trimmed();
    QString spec = ui->specEdit->text();
    QString unit = ui->unitEdit->text();
    double price = ui->priceEdit->text().toDouble();
    int stock = ui->stockQuantityEdit->text().toInt();
    int warning = ui->warningQuantityEdit->text().toInt();
    QString intro = ui->introEdit->toPlainText();

    // 【关键】从下拉框获取选中的仓库ID
    int w_id = ui->wareBox->currentData().toInt();

    // 2. 基础校验
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "货品名称不能为空！");
        return; // 阻止关闭窗口
    }
    if (ui->wareBox->currentIndex() == -1) {
        QMessageBox::warning(this, "提示", "请选择一个仓库！");
        return;
    }

    // 3. 计算总价值
    double totalValue = price * stock;

    // 4. 执行数据库插入
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO goods (
            goods_name, goods_spec, goods_unit, goods_price,
            total_value, goods_intro, stock_quantity, warning_quantity, w_id
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?, ?
        )
    )");

    query.addBindValue(name);
    query.addBindValue(spec);
    query.addBindValue(unit);
    query.addBindValue(price);
    query.addBindValue(totalValue); // 自动计算的总价
    query.addBindValue(intro);
    query.addBindValue(stock);
    query.addBindValue(warning);
    query.addBindValue(w_id); // 写入正确的仓库ID

    if (query.exec()) {
        // 插入成功，调用父类的 accept()，这会关闭窗口并返回 QDialog::Accepted
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "错误", "添加失败: " + query.lastError().text());
        // 注意：这里不调用 QDialog::accept()，保持窗口打开让用户修改
    }
}
