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

void DataWorker::exportToCsv(const QString &filePath)
{
    // 在子线程必须建立新的数据库连接
    QString connName = QString("Worker_Export_%1").arg((quint64)QThread::currentThreadId());
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_management.db");

        if (db.open()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                // 写入CSV表头
                out << "Name,Spec,Unit,Quantity,Warning\n";

                QSqlQuery query("SELECT goods_name, goods_spec, goods_unit, stock_quantity, warning_quantity FROM goods", db);
                while(query.next()) {
                    out << query.value(0).toString() << ","
                        << query.value(1).toString() << ","
                        << query.value(2).toString() << ","
                        << query.value(3).toInt() << ","
                        << query.value(4).toInt() << "\n";
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

void DataWorker::importFromCsv(const QString &filePath)
{
    QString connName = QString("Worker_Import_%1").arg((quint64)QThread::currentThreadId());
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_management.db");

        if (db.open()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                db.transaction(); // 开启事务加速插入

                QSqlQuery query(db);
                query.prepare("INSERT OR IGNORE INTO goods (goods_name, goods_spec, goods_unit, stock_quantity, warning_quantity) VALUES (?, ?, ?, ?, ?)");

                if(!in.atEnd()) in.readLine(); // 跳过表头

                while(!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList parts = line.split(",");
                    if (parts.size() >= 5) {
                        query.addBindValue(parts[0].trimmed()); // Name
                        query.addBindValue(parts[1].trimmed()); // Spec
                        query.addBindValue(parts[2].trimmed()); // Unit
                        query.addBindValue(parts[3].toInt());   // Qty
                        query.addBindValue(parts[4].toInt());   // Warning
                        query.exec();
                    }
                }

                db.commit();
                file.close();
                emit taskFinished("导入成功！");
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
