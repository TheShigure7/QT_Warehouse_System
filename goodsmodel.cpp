#include "goodsmodel.h"
#include <QColor>
#include <QSqlRecord>

// 1. 构造函数初始化列表这里要改：
GoodsModel::GoodsModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)  // <--- 这里的类名必须和头文件继承的类名一致
{
}

QVariant GoodsModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
        int qtyIndex = fieldIndex("stock_quantity");
        int warnIndex = fieldIndex("warning_quantity");

        // 简单的安全检查
        if (qtyIndex != -1 && warnIndex != -1) {
            int currentStock = record(index.row()).value(qtyIndex).toInt();
            int warningStock = record(index.row()).value(warnIndex).toInt();

            if (currentStock < warningStock) {
                return QColor(255, 200, 200);
            }
        }
    }

    // 2. 这里也要改，调用新基类的 data 方法：
    return QSqlRelationalTableModel::data(index, role); // <--- 改成 QSqlRelationalTableModel
}
