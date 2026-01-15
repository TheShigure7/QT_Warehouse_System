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
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/warehouse_v2.db"); // 改个名防止冲突

        if (!db.open()) {
            qCritical() << "Database connection failed:" << db.lastError().text();
            return false;
        }

        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON;"); // 开启外键支持

        // 1. 新增：仓库表 (warehouses)
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS warehouses (
                w_id INTEGER PRIMARY KEY AUTOINCREMENT,
                w_name TEXT NOT NULL,
                w_address TEXT,
                w_capacity INTEGER DEFAULT 10000,
                w_count INTEGER DEFAULT 0,
                w_value REAL DEFAULT 0.0
            )
        )");
        // 初始化测试数据：如果仓库表为空，插入两个仓库
        QSqlQuery checkQuery("SELECT count(*) FROM warehouses");
        if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
            query.exec("INSERT INTO warehouses (w_name, w_address) VALUES ('主仓库(北京)', '北京市海淀区')");
            query.exec("INSERT INTO warehouses (w_name, w_address) VALUES ('分仓库(上海)', '上海市浦东新区')");
        }

        // 2. 修改：货品表 (goods)
        // 增加：price(单价), intro(简介), w_id(仓库外键)
        // 注意：去掉了 goods_name 的 UNIQUE 约束，允许不同仓库有同名商品
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS goods (
                goods_id INTEGER PRIMARY KEY AUTOINCREMENT,
                goods_name TEXT NOT NULL,
                goods_spec TEXT,
                goods_unit TEXT,
                goods_price REAL DEFAULT 0.0,   -- 新增：单价
                total_value REAL DEFAULT 0.0,
                goods_intro TEXT,               -- 新增：简介
                w_id INTEGER NOT NULL,          -- 新增：所属仓库ID
                stock_quantity INTEGER NOT NULL DEFAULT 0,
                warning_quantity INTEGER NOT NULL DEFAULT 10,
                FOREIGN KEY (w_id) REFERENCES warehouses(w_id) ON DELETE CASCADE
            )
        )");

        // 3. 修改：记录表 (stock_record)
        // 增加：单价、总价、仓库ID(用于历史追溯)
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS stock_record (
                record_id INTEGER PRIMARY KEY AUTOINCREMENT,
                goods_id INTEGER NOT NULL,
                w_id INTEGER,                   -- 新增：记录当时的仓库ID
                record_type TEXT NOT NULL,
                record_quantity INTEGER NOT NULL,
                single_price REAL,              -- 新增：当时的单价
                total_price REAL,               -- 新增：总价
                record_time DATETIME DEFAULT CURRENT_TIMESTAMP,
                operator TEXT,
                FOREIGN KEY (goods_id) REFERENCES goods(goods_id) ON DELETE CASCADE
            )
        )");

        return true;
    }
};

#endif // DBMANAGER_H
