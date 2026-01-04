#ifndef RECORDSDIALOG_H
#define RECORDSDIALOG_H

#include <QDialog>

namespace Ui {
class RecordsDialog;
}

class RecordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordsDialog(QWidget *parent = nullptr);
    ~RecordsDialog();

private:
    Ui::RecordsDialog *ui;
};

#endif // RECORDSDIALOG_H
