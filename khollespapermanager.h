#ifndef KHOLLESPAPERMANAGER_H
#define KHOLLESPAPERMANAGER_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class KhollesPaperManager;
}

class KhollesPaperManager : public QDialog
{
    Q_OBJECT

public:
    explicit KhollesPaperManager(QSqlDatabase *db, QWidget *parent = 0);
    ~KhollesPaperManager();

private:
    Ui::KhollesPaperManager *ui;
    QSqlDatabase *m_db;
};

#endif // KHOLLESPAPERMANAGER_H
