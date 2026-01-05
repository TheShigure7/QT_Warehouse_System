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
    model->setHeaderData(model->fieldIndex("total_value"), Qt::Horizontal, "总价值(元)");
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
            total_value, goods_intro, stock_quantity, warning_quantity, w_id
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?,?
        )
    )");

    insertQuery.addBindValue(name);
    insertQuery.addBindValue(spec);
    insertQuery.addBindValue(unit);
    insertQuery.addBindValue(price);
    insertQuery.addBindValue(0.0);
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
// 核心通用逻辑：出入库处理
void MainWindow::handleStockChange(bool isInbound)
{
    // 1. 检查是否选中
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) {
        ui->statusbar->showMessage("请先选择一行货品", 3000);
        return;
    }

    // 2. 获取当前选中的 Goods ID
    int row = index.row();
    // 注意：我们要获取 goods_id，这列在数据库里肯定是有的
    // 使用 record(row).value("goods_id") 是安全的
    int goodsId = model->record(row).value("goods_id").toInt();

    // 3. 【重点修改】直接从数据库查询该商品的最新信息 (单价、仓库ID、当前库存)
    // 这样做是为了防止 UI 上的数据滞后，或者 Model 里的关系映射导致读出奇怪的值
    QSqlQuery q;
    q.prepare("SELECT w_id, goods_price, stock_quantity FROM goods WHERE goods_id = ?");
    q.addBindValue(goodsId);

    int warehouseId = 0;
    double price = 0.0;
    int currentStock = 0;

    if (q.exec() && q.next()) {
        warehouseId = q.value("w_id").toInt();
        price = q.value("goods_price").toDouble();
        currentStock = q.value("stock_quantity").toInt();
    } else {
        QMessageBox::critical(this, "错误", "无法查询商品详细信息！");
        return;
    }

    // 4. 校验单价 (如果是 0，提示用户先去设置单价)
    if (price <= 0.0001) {
        int ret = QMessageBox::warning(this, "提示",
                                       "检测到该商品单价为 0，是否继续？\n建议先在表格中修改单价。",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }

    // 5. 计算数值
    int changeQty = ui->spinAmount->value();
    int newQty = isInbound ? (currentStock + changeQty) : (currentStock - changeQty);
    double totalPrice = price * changeQty; // 计算总价

    double newInventoryTotalValue = price * newQty;

    if (newQty < 0) {
        QMessageBox::critical(this, "错误", "库存不足，无法出库！");
        return;
    }

    // 6. 开启事务写入数据库
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query(db);
    bool success = true;

    // A. 更新库存
    query.prepare("UPDATE goods SET stock_quantity = ?, total_value = ? WHERE goods_id = ?");
    query.addBindValue(newQty);
    query.addBindValue(newInventoryTotalValue); // 写入新的总价值
    query.addBindValue(goodsId);


    if (!query.exec()) success = false;

    // B. 插入记录 (这里写入查出来的 warehouseId 和 price，绝对不会是 0 了)
    if (success) {
        query.prepare(R"(
            INSERT INTO stock_record
            (goods_id, w_id, record_type, record_quantity, single_price, total_price, operator)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        )");
        query.addBindValue(goodsId);
        query.addBindValue(warehouseId); // 写入正确的仓库ID
        query.addBindValue(isInbound ? "入库" : "出库");
        query.addBindValue(changeQty);
        query.addBindValue(price);       // 写入正确的单价
        query.addBindValue(totalPrice);  // 写入正确的总价
        query.addBindValue("Admin");
        if (!query.exec()) success = false;
    }

    // 7. 提交或回滚
    if (success) {
        db.commit();
        ui->statusbar->showMessage(isInbound ? "入库成功" : "出库成功", 3000);
        model->select(); // 刷新界面
        emit dbUpdated();
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
