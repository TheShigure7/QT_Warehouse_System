#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "goodsmodel.h"
#include "dataworker.h"
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileDialog>

// === 构造函数 ===
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化 Model
    model = new GoodsModel(this);
    model->setTable("goods");

    // 设置外键关系：将 goods 表的 w_id 列，关联到 warehouses 表的 w_id，并显示 w_name
    int wIdIndex = model->fieldIndex("w_id");
    model->setRelation(wIdIndex, QSqlRelation("warehouses", "w_id", "w_name"));

    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    // 设置表头
    model->setHeaderData(model->fieldIndex("goods_name"), Qt::Horizontal, "货品名称");
    model->setHeaderData(model->fieldIndex("goods_spec"), Qt::Horizontal, "规格");
    model->setHeaderData(model->fieldIndex("goods_intro"), Qt::Horizontal, "简介");
    model->setHeaderData(model->fieldIndex("goods_unit"), Qt::Horizontal, "单位");
    model->setHeaderData(model->fieldIndex("goods_price"), Qt::Horizontal, "单价(元)");
    model->setHeaderData(wIdIndex, Qt::Horizontal, "所属仓库");
    model->setHeaderData(model->fieldIndex("stock_quantity"), Qt::Horizontal, "库存");
    model->setHeaderData(model->fieldIndex("warning_quantity"), Qt::Horizontal, "预警阈值");


    ui->tableView->setModel(model);

    // 启用关系代理，让"所属仓库"变成下拉框
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    ui->tableView->hideColumn(0); // 隐藏ID
    ui->tableView->setColumnWidth(wIdIndex, 120);
    ui->tableView->setColumnWidth(model->fieldIndex("goods_name"), 150);
    ui->tableView->setColumnWidth(model->fieldIndex("goods_intro"), 200);

    // 初始化线程
    workerThread = new QThread(this);
    worker = new DataWorker();
    worker->moveToThread(workerThread);

    // 信号连接
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &DataWorker::taskFinished, this, &MainWindow::onThreadFinished);
    connect(worker, &DataWorker::taskFailed, this, &MainWindow::onThreadError);
    connect(worker, &DataWorker::dataChanged, this, [this](){
        model->select();
    });

    workerThread->start();
}

// === 析构函数 (报错里提到的 ~MainWindow) ===
MainWindow::~MainWindow()
{
    workerThread->quit();
    workerThread->wait();
    delete ui;
}

// === 槽函数实现 ===

void MainWindow::on_btnAdd_clicked()
{
    // 1. 获取默认仓库ID
    QSqlQuery q("SELECT w_id FROM warehouses LIMIT 1");
    int defaultWId = 1;
    if (q.next()) {
        defaultWId = q.value(0).toInt();
    } else {
        QMessageBox::warning(this, "警告", "没有检测到仓库信息，无法创建货品！\n请检查数据库。");
        return;
    }

    // 2. 准备数据
    QString name = "新货品_" + QString::number(QDateTime::currentMSecsSinceEpoch() % 1000);
    QString spec = "常规";
    QString unit = "个";
    double price = 0.0;
    QString intro = "暂无简介"; // 之前可能因为空值问题，这里给个默认值
    int stock = 0;
    int warning = 10;

    // 3. 【核心修改】使用 SQL 直接插入，绕过 Model 的映射坑
    QSqlQuery insertQuery;
    insertQuery.prepare(R"(
        INSERT INTO goods (
            goods_name, goods_spec, goods_unit, goods_price,
            goods_intro, stock_quantity, warning_quantity, w_id
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?
        )
    )");

    insertQuery.addBindValue(name);
    insertQuery.addBindValue(spec);
    insertQuery.addBindValue(unit);
    insertQuery.addBindValue(price);
    insertQuery.addBindValue(intro);
    insertQuery.addBindValue(stock);
    insertQuery.addBindValue(warning);
    insertQuery.addBindValue(defaultWId); // 直接插入整数ID，数据库最喜欢这个

    // 4. 执行并刷新
    if (!insertQuery.exec()) {
        QMessageBox::critical(this, "插入失败",
                              "无法创建新货品。\n错误信息: " + insertQuery.lastError().text());
    } else {
        model->select(); // 重新查询数据库，界面会自动更新
        ui->tableView->scrollToBottom(); // 滚到底部看新加的行
    }
}

void MainWindow::on_btnDel_clicked()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(selection.isEmpty()) return;

    for(int i = selection.count()-1; i >= 0; i--) {
        model->removeRow(selection.at(i).row());
    }
    model->submitAll();
    model->select();
}

// 核心通用逻辑
void MainWindow::handleStockChange(bool isInbound)
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    int goodsId = model->record(row).value("goods_id").toInt();
    int currentQty = model->record(row).value("stock_quantity").toInt();
    double price = model->record(row).value("goods_price").toDouble();
    int warehouseId = model->record(row).value("w_id").toInt();

    int changeQty = ui->spinAmount->value();
    int newQty = isInbound ? (currentQty + changeQty) : (currentQty - changeQty);
    double totalPrice = price * changeQty;

    if (newQty < 0) {
        QMessageBox::critical(this, "错误", "库存不足！");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query(db);
    bool success = true;

    // 更新库存
    query.prepare("UPDATE goods SET stock_quantity = ? WHERE goods_id = ?");
    query.addBindValue(newQty);
    query.addBindValue(goodsId);
    if (!query.exec()) success = false;

    // 插入记录
    if (success) {
        query.prepare(R"(
            INSERT INTO stock_record
            (goods_id, w_id, record_type, record_quantity, single_price, total_price, operator)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        )");
        query.addBindValue(goodsId);
        query.addBindValue(warehouseId);
        query.addBindValue(isInbound ? "入库" : "出库");
        query.addBindValue(changeQty);
        query.addBindValue(price);
        query.addBindValue(totalPrice);
        query.addBindValue("Admin");
        if (!query.exec()) success = false;
    }

    if (success) {
        db.commit();
        ui->statusbar->showMessage("操作成功", 3000);
        model->select();
    } else {
        db.rollback();
        QMessageBox::critical(this, "错误", query.lastError().text());
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

    ui->groupBoxData->setEnabled(false);
    QMetaObject::invokeMethod(worker, "exportToCsv", Qt::QueuedConnection, Q_ARG(QString, path));
}

void MainWindow::on_btnImport_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "导入", "", "CSV Files (*.csv)");
    if(path.isEmpty()) return;

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
