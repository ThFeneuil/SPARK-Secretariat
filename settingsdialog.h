#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QMessageBox>
#include "preferences.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

public slots:
    bool save();
    bool update();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
