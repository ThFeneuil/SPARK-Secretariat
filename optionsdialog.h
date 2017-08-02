#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QQueue>
#include "storedData/class.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QSqlDatabase* db, QWidget *parent = 0);
    ~OptionsDialog();
    bool update_list();
    bool free_classes();

public slots:
    bool selectClass();
    bool optionsModified();

private:
    Ui::OptionsDialog *ui;
    QSqlDatabase *m_db;
    QQueue<Class*> queue_displayedClasses;
    bool m_canSave;
};

#endif // OPTIONSDIALOG_H
