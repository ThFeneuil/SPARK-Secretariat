#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QByteArray>
#include <QTimer>
#include <QtGlobal>
#include "managers/classesmanager.h"
#include "managers/kholleursmanager.h"
#include "aboutitdialog.h"
#include "contactdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openClassesManager();
    void openKholleursManager();
    void openHelp();
    void openAboutIt();

private:
    Ui::MainWindow *ui;
    QStringList args;
};

#endif // MAINWINDOW_H
