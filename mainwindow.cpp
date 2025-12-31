#include "mainwindow.h"
#include "ui_mainwindow.h" // 重要：这是构建自动生成的头文件
#include "goodsmodel.h"
#include "dataworker.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this); // 加载 UI 文件界面

    // 1. 初始化 Model
    model = new GoodsModel(this);
    model->setTable("goods");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    // 设置表头别名 (对应数据库字段)
    model->setHeaderData(1, Qt::Horizontal, "货品名称");
    model->setHeaderData(2, Qt::Horizontal, "规格");
    model->setHeaderData(3, Qt::Horizontal, "单位");
    model->setHeaderData(4, Qt::Horizontal, "当前库存");
    model->setHeaderData(5, Qt::Horizontal, "预警阈值");

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(0); // 隐藏 ID 列

    // 调整列宽
    ui->tableView->setColumnWidth(1, 150);

    // 2. 初始化后台线程
    workerThread = new QThread(this);
    worker = new DataWorker();
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &DataWorker::taskFinished, this, &MainWindow::onThreadFinished);
    connect(worker, &DataWorker::taskFailed, this, &MainWindow::onThreadError);
    connect(worker, &DataWorker::dataChanged, this, [this](){
        model->select(); // 刷新表格
    });

    workerThread->start();
}

MainWindow::~MainWindow()
{
    workerThread->quit();
    workerThread->wait();
    delete ui;
}

// === 按钮槽函数 (Qt Designer 自动连接机制) ===

void MainWindow::on_btnAdd_clicked()
{
    // 插入新行，由用户在表格中填空
    QSqlRecord record = model->record();
    record.setValue("goods_name", "新货品_" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    record.setValue("stock_quantity", 0);
    record.setValue("warning_quantity", 10);
    model->insertRecord(-1, record);
    model->select();
}

void MainWindow::on_btnDel_clicked()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(selection.isEmpty()) return;

    for(int i = selection.count()-1; i >= 0; i--) {
        model->removeRow(selection.at(i).row());
    }
    model->submitAll(); // 提交删除
    model->select();
}

// 处理入库/出库的通用逻辑
void MainWindow::handleStockChange(bool isInbound)
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) {
        ui->statusbar->showMessage("请先选择一行货品", 3000);
        return;
    }

    int row = index.row();
    // 获取当前信息
    int goodsId = model->record(row).value("goods_id").toInt();
    int currentQty = model->record(row).value("stock_quantity").toInt();
    int changeQty = ui->spinAmount->value();

    // 计算新库存
    int newQty = isInbound ? (currentQty + changeQty) : (currentQty - changeQty);

    if (newQty < 0) {
        QMessageBox::critical(this, "错误", "库存不足，无法出库！");
        return;
    }

    // === 开始数据库事务 ===
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query(db);
    bool success = true;

    // 1. 更新 goods 表库存
    query.prepare("UPDATE goods SET stock_quantity = ? WHERE goods_id = ?");
    query.addBindValue(newQty);
    query.addBindValue(goodsId);
    if (!query.exec()) success = false;

    // 2. 插入 stock_record 表记录 (满足截图需求)
    if (success) {
        query.prepare("INSERT INTO stock_record (goods_id, record_type, record_quantity, operator) VALUES (?, ?, ?, ?)");
        query.addBindValue(goodsId);
        query.addBindValue(isInbound ? "入库" : "出库");
        query.addBindValue(changeQty);
        query.addBindValue("Admin"); // 默认操作员
        if (!query.exec()) success = false;
    }

    if (success) {
        db.commit();
        ui->statusbar->showMessage(isInbound ? "入库成功" : "出库成功", 3000);
        model->select(); // 刷新界面
    } else {
        db.rollback();
        QMessageBox::critical(this, "数据库错误", query.lastError().text());
    }
}

void MainWindow::on_btnIn_clicked()
{
    handleStockChange(true);
}

void MainWindow::on_btnOut_clicked()
{
    handleStockChange(false);
}

void MainWindow::on_btnExport_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "导出", "", "CSV Files (*.csv)");
    if(path.isEmpty()) return;

    ui->statusbar->showMessage("正在后台导出...", 0);
    ui->groupBoxData->setEnabled(false); // 防止重复操作

    // 调用后台线程
    QMetaObject::invokeMethod(worker, "exportToCsv", Qt::QueuedConnection, Q_ARG(QString, path));
}

void MainWindow::on_btnImport_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "导入", "", "CSV Files (*.csv)");
    if(path.isEmpty()) return;

    ui->statusbar->showMessage("正在后台导入...", 0);
    ui->groupBoxData->setEnabled(false);

    QMetaObject::invokeMethod(worker, "importFromCsv", Qt::QueuedConnection, Q_ARG(QString, path));
}

void MainWindow::onThreadFinished(const QString &msg)
{
    ui->statusbar->showMessage(msg, 5000);
    ui->groupBoxData->setEnabled(true);
    QMessageBox::information(this, "完成", msg);
}

void MainWindow::onThreadError(const QString &err)
{
    ui->statusbar->showMessage("错误: " + err, 5000);
    ui->groupBoxData->setEnabled(true);
    QMessageBox::critical(this, "错误", err);
}
