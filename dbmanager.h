#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>

class DbManager {
public:
    static bool initDatabase() {
        // 使用 SQLite
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        // 数据库文件路径
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_management.db");

        if (!db.open()) {
            qCritical() << "Database connection failed:" << db.lastError().text();
            return false;
        }

        QSqlQuery query;

        // 1. 货品信息表 (goods) - 对应截图
        bool success = query.exec(R"(
            CREATE TABLE IF NOT EXISTS goods (
                goods_id INTEGER PRIMARY KEY AUTOINCREMENT,
                goods_name TEXT NOT NULL UNIQUE,
                goods_spec TEXT,
                goods_unit TEXT,
                stock_quantity INTEGER NOT NULL DEFAULT 0,
                warning_quantity INTEGER NOT NULL DEFAULT 10
            )
        )");
        if (!success) qCritical() << "Create goods table error:" << query.lastError();

        // 2. 出入库记录表 (stock_record) - 对应截图
        success = query.exec(R"(
            CREATE TABLE IF NOT EXISTS stock_record (
                record_id INTEGER PRIMARY KEY AUTOINCREMENT,
                goods_id INTEGER NOT NULL,
                record_type TEXT NOT NULL,
                record_quantity INTEGER NOT NULL,
                record_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
                operator TEXT,
                FOREIGN KEY (goods_id) REFERENCES goods(goods_id) ON DELETE CASCADE
            )
        )");
        if (!success) qCritical() << "Create stock_record table error:" << query.lastError();

        // 3. 开启外键支持
        query.exec("PRAGMA foreign_keys = ON;");

        return true;
    }
};

#endif // DBMANAGER_H
