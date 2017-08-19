#ifndef KHOLLEURSMANAGER_H
#define KHOLLEURSMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QQueue>
#include "storedData/kholleur.h"
#include "managers/updatekholleurdialog.h"

namespace Ui {
class KholleursManager;
}

class KholleursManager : public QDialog
{
    Q_OBJECT

public:
    explicit KholleursManager(QSqlDatabase *db, QWidget *parent = 0);
    ~KholleursManager();
    bool free_kholleurs();

public slots:
    bool update_list();
    bool add_kholleur();
    bool delete_kholleur();
    bool update_kholleur(QListWidgetItem *item);

private:
    Ui::KholleursManager *ui;
    QSqlDatabase *m_db;
    QQueue<Kholleur*> queue_displayedKholleurs;
};

#endif // KHOLLEURSMANAGER_H
