#include "warehousedialog.h"
#include "ui_warehousedialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

WarehouseDialog::WarehouseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarehouseDialog)
{
    ui->setupUi(this);

    // 默认新增模式
    m_id = -1;
    ui->capSpin1->setValue(10000); // 给个默认容量
}

WarehouseDialog::~WarehouseDialog()
{
    delete ui;
}

// 加载数据 (修改模式用)
void WarehouseDialog::loadData(int w_id)
{
    m_id = w_id; // 标记ID
    this->setWindowTitle("修改仓库信息");

    QSqlQuery q;
    q.prepare("SELECT * FROM warehouses WHERE w_id = ?");
    q.addBindValue(w_id);

    if (q.exec() && q.next()) {
        ui->nameEdit->setText(q.value("w_name").toString());
        ui->addrEdit->setText(q.value("w_address").toString());
        ui->capSpin1->setValue(q.value("w_capacity").toInt());
    }
}

// 点击确定
void WarehouseDialog::on_btnOk_clicked()
{
    QString name = ui->nameEdit->text().trimmed();
    QString addr = ui->addrEdit->toPlainText().trimmed();
    int capacity = ui->capSpin1->value();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "仓库名称不能为空！");
        return;
    }

    QSqlQuery q;
    bool success = false;

    if (m_id == -1) {
        // === 新增模式 ===
        // 注意：w_count(总数) 和 w_value(总价) 初始为 0
        q.prepare("INSERT INTO warehouses (w_name, w_address, w_capacity, w_count, w_value) VALUES (?, ?, ?, 0, 0)");
        q.addBindValue(name);
        q.addBindValue(addr);
        q.addBindValue(capacity);
    } else {
        // === 修改模式 ===
        // 注意：不要去更新 count 和 value，那些是统计出来的
        q.prepare("UPDATE warehouses SET w_name=?, w_address=?, w_capacity=? WHERE w_id=?");
        q.addBindValue(name);
        q.addBindValue(addr);
        q.addBindValue(capacity);
        q.addBindValue(m_id);
    }

    success = q.exec();

    if (success) {
        accept(); // 关闭窗口返回成功
    } else {
        QMessageBox::critical(this, "错误", "操作失败: " + q.lastError().text());
    }
}

// 点击取消
void WarehouseDialog::on_btnCancel_clicked()
{
    reject();
}
