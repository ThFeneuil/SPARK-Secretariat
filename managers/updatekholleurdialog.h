#ifndef UPDATEKHOLLEURDIALOG_H
#define UPDATEKHOLLEURDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QQueue>
#include "storedData/kholleur.h"

namespace Ui {
class UpdateKholleurDialog;
}

class UpdateKholleurDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateKholleurDialog(QSqlDatabase *db, Kholleur *khll, QWidget *parent = 0);
    ~UpdateKholleurDialog();

public slots:
    bool update_kholleur();

private:
    Ui::UpdateKholleurDialog *ui;
    QSqlDatabase *m_db;
    Kholleur *m_kholleur;
};

#endif // UPDATEKHOLLEURDIALOG_H
