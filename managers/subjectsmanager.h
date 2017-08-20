#ifndef SUBJECTSMANAGER_H
#define SUBJECTSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "storedData/subject.h"

namespace Ui {
class SubjectsManager;
}

class SubjectsManager : public QDialog
{
    Q_OBJECT

public:
    explicit SubjectsManager(QSqlDatabase *db, QWidget *parent = 0);
    ~SubjectsManager();
    bool free_subjects();

public slots:
    bool update_list();
    bool add_subject();
    bool delete_subject();

private:
    Ui::SubjectsManager *ui;
    QSqlDatabase *m_db;
    QQueue<Subject*> queue_displayedSubjects;
};

#endif // SUBJECTSMANAGER_H
