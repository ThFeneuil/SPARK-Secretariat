#ifndef CLASSESMANAGER_H
#define CLASSESMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "storedData/class.h"

namespace Ui {
class ClassesManager;
}

class ClassesManager : public QDialog
{
    Q_OBJECT

public:
    explicit ClassesManager(QSqlDatabase *db, QWidget *parent = 0);
    ~ClassesManager();
    bool free_classes();

public slots:
    bool update_list();
    bool add_class();
    bool delete_class();

private:
    Ui::ClassesManager *ui;
    QSqlDatabase *m_db;
    QQueue<Class*> queue_displayedClasses;
};

#endif // KHOLLEURSMANAGER_H
