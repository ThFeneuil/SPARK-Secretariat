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
#include "preferences.h"

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
    void writeDiffusionHistory(QString text);
    void finishedDiffusion();

public slots:
    void diffuse();
    void requestReturn(ODBRequest *req);
    void infoLabel();
    void update_list_mondays();
    void update_edit_monday();

private:
    Ui::DiffusionManager *ui;

    QMap<int, Class*> m_classes;
    QMap<int, Kholleur*> m_kholleurs;
    QMap<int, Subject*> m_subjects;

    // Diffusion
    int m_byServer_nbTotal;
    int m_byServer_nbReceived;
    bool m_byPaper_built;
    int m_nbErrors;
};

#endif // DIFFUSIONMANAGER_H
