#ifndef RECORDSDIALOG_H
#define RECORDSDIALOG_H

#include <QDialog>
#include <QSqlRelationalTableModel> // 注意这里引用的是 Relational 模型
#include <QThread> // 新增
#include "dataworker.h" // 新增

namespace Ui {
class RecordsDialog;
}

class RecordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordsDialog(QWidget *parent = nullptr);
    ~RecordsDialog();
    void refreshData();

private slots:
    void on_btnExportRec_clicked(); // 新增按钮槽函数

    // 线程回调
    void onWorkerFinished(const QString &msg);



private:
    Ui::RecordsDialog *ui;
    QSqlRelationalTableModel *model; // 使用关系型模型

    // 增加线程相关对象
    QThread *workerThread;
    DataWorker *worker;
};

#endif // RECORDSDIALOG_H
