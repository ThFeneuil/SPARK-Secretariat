#ifndef KHOLLESPAPERMANAGER_H
#define KHOLLESPAPERMANAGER_H

#include <QDialog>
#include <QtSql>
#include "selectioninterface.h"
#include "printpdf.h"

namespace Ui {
class KhollesPaperManager;
}

class KhollesPaperManager : public QDialog
{
    Q_OBJECT

public:
    explicit KhollesPaperManager(QSqlDatabase *db, QWidget *parent = 0);
    ~KhollesPaperManager();

public slots:
    void printKhollesPaper();

private:
    Ui::KhollesPaperManager *ui;
    QSqlDatabase *m_db;
    SelectionInterface *m_interface;
};

#endif // KHOLLESPAPERMANAGER_H
