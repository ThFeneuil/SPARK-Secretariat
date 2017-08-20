#ifndef DIFFUSIONMANAGER_H
#define DIFFUSIONMANAGER_H

#include <QDialog>
#include <QtSql>
#include <QDebug>
#include <QListWidgetItem>
#include "onlinedatabase.h"
#include "printpdf.h"
#include "storedData/class.h"
#include "storedData/kholleur.h"
#include "storedData/subject.h"

namespace Ui {
class DiffusionManager;
}

class DiffusionManager : public QDialog
{
    Q_OBJECT

public:
    explicit DiffusionManager(QWidget *parent = 0);
    ~DiffusionManager();
    bool diffuseServer(Class* cls);

public slots:
    void diffuse();
    void test(ODBRequest *req);
    void infoLabel();

private:
    Ui::DiffusionManager *ui;

    QMap<int, Class*> m_classes;
    QMap<int, Kholleur*> m_kholleurs;
    QMap<int, Subject*> m_subjects;
};

#endif // DIFFUSIONMANAGER_H
