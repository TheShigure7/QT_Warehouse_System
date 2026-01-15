#ifndef DATAWORKER_H
#define DATAWORKER_H

#include <QObject>

class DataWorker : public QObject
{
    Q_OBJECT
public:
    explicit DataWorker(QObject *parent = nullptr);

public slots:
    void exportToCsv(const QString &filePath);
    void exportRecordsToCsv(const QString &filePath); // 【新增】导出记录
    void importFromCsv(const QString &filePath);

signals:
    void taskFinished(const QString &msg);
    void taskFailed(const QString &err);
    void dataChanged(); // 通知UI刷新
};

#endif // DATAWORKER_H
