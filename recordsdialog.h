#ifndef RECORDSDIALOG_H
#define RECORDSDIALOG_H

#include <QDialog>
#include <QSqlRelationalTableModel> // 注意这里引用的是 Relational 模型

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

private:
    Ui::RecordsDialog *ui;
    QSqlRelationalTableModel *model; // 使用关系型模型
};

#endif // RECORDSDIALOG_H
