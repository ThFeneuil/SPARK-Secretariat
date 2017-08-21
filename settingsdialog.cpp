#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    Preferences pref;
    bool isDefault = pref.serverDefault();

    ui->radioButton_default->setChecked(isDefault);
    ui->radioButton_other->setChecked(! isDefault);

    ui->lineEdit_script->setEnabled(! isDefault);
    ui->lineEdit_password->setEnabled(! isDefault);
    ui->lineEdit_script->setText(pref.serverScript());
    ui->lineEdit_password->setText(pref.serverPassword());

    connect(ui->pushButton_valid, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(ui->radioButton_other, SIGNAL(toggled(bool)), this, SLOT(update()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::update() {
    ui->lineEdit_script->setEnabled(ui->radioButton_other->isChecked());
    ui->lineEdit_password->setEnabled(ui->radioButton_other->isChecked());
    return true;
}

bool SettingsDialog::save() {
    Preferences pref;
    pref.setServerDefault(ui->radioButton_default->isChecked());
    pref.setServerScript(ui->lineEdit_script->text());
    pref.setServerPassword(ui->lineEdit_password->text());
    accept();
    return true;
}

