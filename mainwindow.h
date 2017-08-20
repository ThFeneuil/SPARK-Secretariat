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
#include "managers/classesmanager.h"
#include "managers/kholleursmanager.h"
#include "managers/subjectsmanager.h"
#include "aboutitdialog.h"
#include "contactdialog.h"
#include "weekbox.h"
#include "optionsdialog.h"
#include "diffusionmanager.h"
#include "printpdf.h"
#include "printdialog.h"

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
    void updateWindow();
    void displayDurations();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

public slots:
    void openClassesManager();
    void openKholleursManager();
    void openSubjectsManager();
    void openOptions();
    void openDiffusionManager();
    void openPrintDialog();
    void openHelp();
    void openAboutIt();
    void selectKholleur(QString name);
    void middleAreaEmpty(bool noKholleur, bool noClass);
    void selectClass(QString name);
    void selectInList(QString name, QListWidget* list, TypeElement type);
    void displayLists();
    void kholleurSelected();
    void addWeek();
    void saveDurations();
    void saveDurationsAll();

private:
    Ui::MainWindow *ui;
    QStringList args;
    QList<Kholleur*>* m_list_kholleurs;
    QList<Class*>* m_list_classes;
    QList<Subject*>* m_list_subjects;
    QList<WeekBox*>* m_weekboxes;
    QDate m_firstMonday;
    QSqlDatabase* m_db;
};

#endif // MAINWINDOW_H
