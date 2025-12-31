#include "goodsmodel.h"
#include <QColor>
#include <QSqlRecord>

GoodsModel::GoodsModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db) {}

QVariant GoodsModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
        // 获取当前行的库存和预警值
        int qtyIndex = fieldIndex("stock_quantity");
        int warnIndex = fieldIndex("warning_quantity");

        // 安全检查
        if (qtyIndex == -1 || warnIndex == -1)
            return QSqlTableModel::data(index, role);

        int currentStock = record(index.row()).value(qtyIndex).toInt();
        int warningStock = record(index.row()).value(warnIndex).toInt();

        // 库存不足预警（红色背景）
        if (currentStock < warningStock) {
            return QColor(255, 200, 200);
        }
    }
    return QSqlTableModel::data(index, role);
}
