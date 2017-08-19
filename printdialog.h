#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>
#include "printpdf.h"

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = 0);
    ~PrintDialog();

public slots:
    void print();

private:
    Ui::PrintDialog *ui;
};

#endif // PRINTDIALOG_H
