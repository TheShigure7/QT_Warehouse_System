#include "dataworker.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QCoreApplication>

DataWorker::DataWorker(QObject *parent) : QObject(parent) {}

// === 导出功能 (更新版) ===
void DataWorker::exportToCsv(const QString &filePath)
{
    // 1. 建立线程独立的数据库连接
    QString connName = QString("Worker_Export_%1").arg((quint64)QThread::currentThreadId());
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_v2.db");

        if (db.open()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                // 使用 BOM 防止 Excel 打开中文乱码
                QByteArray bom;
                bom.append(char(0xEF));
                bom.append(char(0xBB));
                bom.append(char(0xBF));
                file.write(bom);

                QTextStream out(&file);
                // 写入表头 (对应新结构)
                out << "货品名称,规格,单位,单价,所属仓库,库存数量,预警阈值,简介\n";

                // 联合查询，把 w_id 变成仓库名字导出
                QSqlQuery query(db);
                query.exec(R"(
                    SELECT g.goods_name, g.goods_spec, g.goods_unit, g.goods_price,
                           w.w_name, g.stock_quantity, g.warning_quantity, g.goods_intro
                    FROM goods g
                    LEFT JOIN warehouses w ON g.w_id = w.w_id
                )");

                while(query.next()) {
                    out << query.value(0).toString() << ","
                        << query.value(1).toString() << ","
                        << query.value(2).toString() << ","
                        << query.value(3).toDouble() << ","
                        << query.value(4).toString() << ","  // 仓库名称
                        << query.value(5).toInt() << ","
                        << query.value(6).toInt() << ","
                        << query.value(7).toString() << "\n";
                }
                file.close();
                emit taskFinished("导出成功！");
            } else {
                emit taskFailed("无法写入文件");
            }
            db.close();
        } else {
            emit taskFailed("数据库连接失败");
        }
    }
    QSqlDatabase::removeDatabase(connName);
}

// === 导入功能 (更新版) ===
void DataWorker::importFromCsv(const QString &filePath)
{
    QString connName = QString("Worker_Import_%1").arg((quint64)QThread::currentThreadId());
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_v2.db");

        if (db.open()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);

                // 开启事务 (极大提升速度)
                db.transaction();

                QSqlQuery query(db);
                // 准备插入语句 (包含 total_value)
                query.prepare(R"(
                    INSERT INTO goods (
                        goods_name, goods_spec, goods_unit, goods_price,
                        w_id, stock_quantity, warning_quantity, goods_intro, total_value
                    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
                )");

                // 跳过第一行表头
                if(!in.atEnd()) in.readLine();

                int successCount = 0;
                while(!in.atEnd()) {
                    QString line = in.readLine();
                    if (line.trimmed().isEmpty()) continue;

                    // 简单的 CSV 解析 (假设没有逗号在字段内容里)
                    QStringList parts = line.split(",");

                    // 确保列数足够 (至少8列)
                    if (parts.size() >= 8) {
                        QString name = parts[0].trimmed();
                        QString spec = parts[1].trimmed();
                        QString unit = parts[2].trimmed();
                        double price = parts[3].toDouble();
                        QString wName = parts[4].trimmed(); // CSV里是仓库名
                        int qty = parts[5].toInt();
                        int warn = parts[6].toInt();
                        QString intro = parts[7].trimmed();

                        // 1. 处理仓库 ID (根据名字查ID)
                        // 如果 CSV 里的仓库名在数据库找不到，默认归到 ID 1
                        int w_id = 1;
                        QSqlQuery wQuery(db);
                        wQuery.prepare("SELECT w_id FROM warehouses WHERE w_name = ?");
                        wQuery.addBindValue(wName);
                        if (wQuery.exec() && wQuery.next()) {
                            w_id = wQuery.value(0).toInt();
                        }

                        // 2. 自动计算总价值
                        double totalValue = price * qty;

                        // 3. 绑定参数
                        query.addBindValue(name);
                        query.addBindValue(spec);
                        query.addBindValue(unit);
                        query.addBindValue(price);
                        query.addBindValue(w_id);
                        query.addBindValue(qty);
                        query.addBindValue(warn);
                        query.addBindValue(intro);
                        query.addBindValue(totalValue); // 写入总价

                        if(query.exec()) successCount++;
                    }
                }

                db.commit(); // 提交事务
                file.close();

                // 触发一次统计更新 (这里只能简单处理，或者让主界面去刷新)
                emit taskFinished(QString("导入成功！共导入 %1 条数据").arg(successCount));
                emit dataChanged();
            } else {
                emit taskFailed("无法读取文件");
            }
            db.close();
        } else {
            emit taskFailed("数据库连接失败");
        }
    }
    QSqlDatabase::removeDatabase(connName);
}
