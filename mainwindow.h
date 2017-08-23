#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QByteArray>
#include <QTimer>
#include <QtGlobal>
#include <QDebug>
#include <QList>
#include <QListWidget>
#include <QKeyEvent>
#include <QTimer>
#include "managers/classesmanager.h"
#include "managers/kholleursmanager.h"
#include "managers/subjectsmanager.h"
#include "storedData/parameterskholleurclass.h"
#include "aboutitdialog.h"
#include "weekbox.h"
#include "optionsdialog.h"
#include "diffusionmanager.h"
#include "printpdf.h"
#include "khollespapermanager.h"
#include "preferences.h"
#include "settingsdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum TypeElement { TYPE_KHOLLEUR, TYPE_CLASS };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int distanceLevenshtein(QString u, QString v, int dmax, int cInsert = 1, int cDelete = 1);
    void initListsKholleursClassesSubjects();
    void initListKholleurs();
    void initListClasses();
    void initListSubjects();
    void updateWindow();
    void displayDurations();
    void loadParametersKholleursClasses();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

public slots:
    void openClassesManager();
    void openKholleursManager();
    void openSubjectsManager();
    void openOptions();
    void openDiffusionManager();
    void openKhollesPaperManager();
    void openAboutIt();
    void selectKholleur(QString name);
    void selectClass(QString name);
    void waitAndSelectInList(QString name, QListWidget* list, TypeElement type);
    void selectInList();
    void displayLists();
    void kholleurSelected();
    void addWeek();
    void saveDurations();
    void saveDurationsAll();
    void openSEC(bool withPref = false);
    void createSEC();
    void openSettings();
    void saveSelection();
    void putSavedSelection();

private:
    Ui::MainWindow *ui;
    QStringList args;
    QList<Kholleur*>* m_list_kholleurs;
    QList<Class*>* m_list_classes;
    QList<Subject*>* m_list_subjects;
    QList<WeekBox*>* m_weekboxes;
    QDate m_firstMonday;
    QSqlDatabase* m_db;
    QMap<int, QMap<int, ParametersKholleurClass>> m_paraKC;
    int m_saveIdSelecKholleur;
    int m_saveIdSelecClass;

    QTimer* m_researchTimer;
    QString m_researchName;
    QListWidget* m_researchList;
    TypeElement m_researchType;
};

#endif // MAINWINDOW_H
