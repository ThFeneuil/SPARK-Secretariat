#ifndef SELECTIONINTERFACE_H
#define SELECTIONINTERFACE_H

#include <QWidget>
#include <QtSql>
#include <QDebug>
#include <QListWidgetItem>
#include <QMessageBox>
#include "storedData/class.h"
#include "storedData/kholleur.h"
#include "storedData/subject.h"

namespace Ui {
class SelectionInterface;
}

class SelectionInterface : public QWidget
{
    Q_OBJECT

public:
    explicit SelectionInterface(QSqlDatabase* db, QWidget *parent = 0);
    ~SelectionInterface();
    QList<QListWidgetItem*> classesSelectedItems();
    QDate date();

public slots:
    void update_list_mondays();
    void update_edit_monday();
    void boldSelection(QListWidgetItem* current, QListWidgetItem* previous);
    void itemSelectionChanged();

signals:
    void classesSelectionChanged();

private:
    Ui::SelectionInterface *ui;

    QSqlDatabase* m_db;

    QMap<int, Class*> m_classes;
    QMap<int, Kholleur*> m_kholleurs;
    QMap<int, Subject*> m_subjects;
};

#endif // SELECTIONINTERFACE_H
