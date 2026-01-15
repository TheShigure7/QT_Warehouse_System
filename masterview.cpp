#include "masterview.h"
#include "ui_masterview.h"
#include "dbmanager.h" // 确保数据库已初始化

MasterView::MasterView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MasterView)
{
    ui->setupUi(this);

    // 1. 初始化数据库 (如果 main.cpp 里没调的话，这里最好调一次)
    DbManager::initDatabase();

    // 2. 加载子页面
    initViews();
}

MasterView::~MasterView()
{
    delete ui;
}

void MasterView::initViews()
{
    // === 页面 1: 货品管理 (MainWindow) ===
    pageGoods = new MainWindow(this);
    // MainWindow 自带菜单栏和状态栏，嵌入后会自动显示在内部区域

    // === 页面 2: 出入库记录 (RecordsDialog) ===
    pageRecords = new RecordsDialog(this);
    // 【关键】RecordsDialog 原本是弹窗，必须设为 Widget 模式才能嵌入
    pageRecords->setWindowFlags(Qt::Widget);

    pageWarehouse = new WarehousePage(this);

    // === 添加到 StackedWidget ===
    // 索引 0
    ui->stackedWidget->addWidget(pageGoods);
    // 索引 1
    ui->stackedWidget->addWidget(pageRecords);

    ui->stackedWidget->addWidget(pageWarehouse);

    // 默认显示第 0 页
    ui->stackedWidget->setCurrentIndex(0);

    connect(pageGoods, &MainWindow::dbUpdated,
            pageRecords, &RecordsDialog::refreshData);
}

// 切换到“货品管理”
void MasterView::on_btnPageGoods_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

// 切换到“出入库记录”
void MasterView::on_btnPageRecords_clicked()
{
    // 如果需要每次切过来都刷新数据，可以调用 select()
    // pageRecords->refreshData(); // 需要你在 RecordsDialog 可以在 public 里加个刷新函数

    ui->stackedWidget->setCurrentIndex(1);
}

// 切换到“仓库信息”
void MasterView::on_btnPageWarehouse_clicked()
{
    // 切换到索引 2
    ui->stackedWidget->setCurrentIndex(2);

    // 可选：每次切过来都刷新一下数据
    pageWarehouse->refreshData();
}
