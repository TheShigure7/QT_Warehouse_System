#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "goodstable.h"

// 引入 UI 命名空间
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GoodsModel;
class DataWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

     void setCurrentUser(const QString &user) { m_currentUser = user; }


signals:
    void dbUpdated(); // 定义一个信号-数据库已更新

private slots:
    // 界面按钮槽函数
    void on_btnAdd_clicked();
    void on_btnDel_clicked();
    void on_btnIn_clicked();  // 入库
    void on_btnOut_clicked(); // 出库
    void on_btnImport_clicked();
    void on_btnExport_clicked();
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void on_btnPopAdd_clicked();
    void on_btnPopEdit_clicked();
    void on_tabWidget_currentChanged(int index);

    void updateWarehouseStats(int w_id);

    // 线程回调
    void onThreadFinished(const QString &msg);
    void onThreadError(const QString &err);

private:
    Ui::MainWindow *ui; // 指向 .ui 文件的指针
    GoodsModel *model;
    QThread *workerThread;
    DataWorker *worker;

    // 辅助函数：处理出入库逻辑
    void handleStockChange(bool isInbound);

    // 【新增】初始化底部标签栏
    void initWarehouseTabs();

    // 【新增】用来存储 Tab 索引和仓库 ID 的对应关系
    // 例如: index 0 -> -1 (全部), index 1 -> 1 (主仓库), index 2 -> 2 (分仓库)
    QList<int> m_tabWarehouseIds;

    QString m_currentUser; // 【新增】存储当前登录的用户名
};

#endif // MAINWINDOW_H
