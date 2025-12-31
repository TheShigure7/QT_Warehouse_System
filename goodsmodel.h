#ifndef GOODSMODEL_H
#define GOODSMODEL_H

#include <QSqlTableModel>

class GoodsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit GoodsModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    // 核心：重写 data 函数处理颜色
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // GOODSMODEL_H
