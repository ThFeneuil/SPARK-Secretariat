#include "updatekholleurdialog.h"
#include "ui_updatekholleurdialog.h"

UpdateKholleurDialog::UpdateKholleurDialog(QSqlDatabase *db, Kholleur *khll, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateKholleurDialog)
{
    // Initialisation
    ui->setupUi(this);
    m_db = db;
    m_kholleur = khll;

    // Set the input field with the data of the kholleur
    ui->lineEdit_name->setText(m_kholleur->getName());

    // Connect the button to update
    connect(ui->pushButton_update, SIGNAL(clicked()), this, SLOT(update_kholleur()));
}

UpdateKholleurDialog::~UpdateKholleurDialog() {
    delete ui;
}

bool UpdateKholleurDialog::update_kholleur() {
    // Get the parameters given par the user
    QString name = ui->lineEdit_name->text();

    if(name == "") { //If there is no name...
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du kholleur.");
        return false;
    } else { // Else update the kholleur
        QSqlQuery query(*m_db);
        query.prepare("UPDATE sec_kholleurs SET name=:name WHERE id=:id");
        query.bindValue(":name", name);
        query.bindValue(":id", m_kholleur->getId());
        query.exec();

        accept();
    }

    return true;
}

