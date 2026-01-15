#include "recordsdialog.h"
#include "ui_recordsdialog.h"
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QFileDialog>
#include <QMessageBox>

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
    model->setHeaderData(model->fieldIndex("operator"), Qt::Horizontal, "操作员");

    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));

    ui->tableView->hideColumn(0);
    ui->tableView->resizeColumnsToContents();

    // 【新增】初始化多线程 Worker
    workerThread = new QThread(this);
    worker = new DataWorker(); // 这里不需要 parent，否则无法 moveToThread
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    // 只连接 taskFinished 即可，导出不需要 dataChanged 信号
    connect(worker, &DataWorker::taskFinished, this, &RecordsDialog::onWorkerFinished);

    workerThread->start();
}

// === 析构函数 (必须要有这个！) ===
RecordsDialog::~RecordsDialog()
{
    // 【新增】安全退出线程
    workerThread->quit();
    workerThread->wait();
    delete ui;
}

void RecordsDialog::refreshData()
{
    // 重新查询数据库
    model->select();

    // 可选：如果是按时间倒序排列，可以在这里重新应用一下排序
    // model->sort(model->fieldIndex("record_time"), Qt::DescendingOrder);
}

// 【新增】导出按钮点击事件
void RecordsDialog::on_btnExportRec_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "导出记录", "", "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    // 禁用按钮防止重复点击
    ui->btnExportRec->setEnabled(false);
    ui->tableView->setEnabled(false); // 可选：导出时暂时禁用表格

    // 调用后台线程的新方法
    QMetaObject::invokeMethod(worker, "exportRecordsToCsv",
                              Qt::QueuedConnection, Q_ARG(QString, path));
}

// 【新增】导出完成回调
void RecordsDialog::onWorkerFinished(const QString &msg)
{
    ui->btnExportRec->setEnabled(true);
    ui->tableView->setEnabled(true);
    QMessageBox::information(this, "导出完成", msg);
}


