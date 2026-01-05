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



// 【新增】加载旧数据到界面
void GoodsTable::setEditData(int goodsId)
{
    m_goodsId = goodsId; // 记录ID
    this->setWindowTitle("修改货品档案"); // 改一下窗口标题

    QSqlQuery query;
    query.prepare("SELECT * FROM goods WHERE goods_id = ?");
    query.addBindValue(goodsId);

    if (query.exec() && query.next()) {
        ui->nameEdit->setText(query.value("goods_name").toString());
        ui->specEdit->setText(query.value("goods_spec").toString());
        ui->unitEdit->setText(query.value("goods_unit").toString());
        ui->priceEdit->setText(query.value("goods_price").toString());
        ui->stockQuantityEdit->setText(query.value("stock_quantity").toString());
        ui->warningQuantityEdit->setText(query.value("warning_quantity").toString());
        ui->introEdit->setText(query.value("goods_intro").toString());

        // 【关键】下拉框回显
        int w_id = query.value("w_id").toInt();
        // findData 可以根据绑定的ID找到对应的索引
        int index = ui->wareBox->findData(w_id);
        if (index != -1) {
            ui->wareBox->setCurrentIndex(index);
        }
    }
}

// 【修改】点击确定时的逻辑
void GoodsTable::accept()
{
    // 1. 获取输入数据 (这些跟之前一样)
    QString name = ui->nameEdit->text().trimmed();
    QString spec = ui->specEdit->text();
    QString unit = ui->unitEdit->text();
    double price = ui->priceEdit->text().toDouble();
    int stock = ui->stockQuantityEdit->text().toInt();
    int warning = ui->warningQuantityEdit->text().toInt();
    QString intro = ui->introEdit->toPlainText();
    int w_id = ui->wareBox->currentData().toInt();
    double totalValue = price * stock; // 重新计算总价

    // 基础校验
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "货品名称不能为空！");
        return;
    }
    if (ui->wareBox->currentIndex() == -1) {
        QMessageBox::warning(this, "提示", "请选择一个仓库！");
        return;
    }

    QSqlQuery query;

    // 2. 判断是新增还是修改
    if (m_goodsId == -1) {
        // === 新增模式 (INSERT) ===
        query.prepare(R"(
            INSERT INTO goods (
                goods_name, goods_spec, goods_unit, goods_price,
                total_value, goods_intro, stock_quantity, warning_quantity, w_id
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");
        // 绑定9个参数
        query.addBindValue(name);
        query.addBindValue(spec);
        query.addBindValue(unit);
        query.addBindValue(price);
        query.addBindValue(totalValue);
        query.addBindValue(intro);
        query.addBindValue(stock);
        query.addBindValue(warning);
        query.addBindValue(w_id);

    } else {
        // === 修改模式 (UPDATE) ===
        query.prepare(R"(
            UPDATE goods SET
                goods_name=?, goods_spec=?, goods_unit=?, goods_price=?,
                total_value=?, goods_intro=?, stock_quantity=?, warning_quantity=?, w_id=?
            WHERE goods_id=?
        )");
        // 绑定10个参数 (最后多一个 ID)
        query.addBindValue(name);
        query.addBindValue(spec);
        query.addBindValue(unit);
        query.addBindValue(price);
        query.addBindValue(totalValue);
        query.addBindValue(intro);
        query.addBindValue(stock);
        query.addBindValue(warning);
        query.addBindValue(w_id);
        query.addBindValue(m_goodsId); // WHERE 条件
    }

    // 3. 执行
    if (query.exec()) {
        QDialog::accept(); // 关闭并返回成功
    } else {
        QMessageBox::critical(this, "错误", "操作失败: " + query.lastError().text());
    }
}
