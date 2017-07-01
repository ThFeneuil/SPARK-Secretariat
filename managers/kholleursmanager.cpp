#include "managers/kholleursmanager.h"
#include "ui_kholleursmanager.h"

KholleursManager::KholleursManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KholleursManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_kholleur()));
    connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(delete_kholleur()));

    // DB
    m_db = db;
    update_list();
}

KholleursManager::~KholleursManager() {
    delete ui;
    free_kholleurs();
}

bool KholleursManager::free_kholleurs() {
    while (!queue_displayedKholleurs.isEmpty())
        delete queue_displayedKholleurs.dequeue();
    return true;
}

bool KholleursManager::update_list() {
    // Clear the list
    ui->list_kholleurs->clear();
    free_kholleurs();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM sec_kholleurs ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(khll->getName(), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
        queue_displayedKholleurs.enqueue(khll);
    }

    return true;
}

bool KholleursManager::add_kholleur() {
    QString name = ui->lineEdit_name->text();

    if(name == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom du kholleur.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO sec_kholleurs(name) VALUES(:name)");
        query.bindValue(":name", name);
        query.exec();

        ui->lineEdit_name->clear();

        update_list();
    }

    return true;
}

bool KholleursManager::delete_kholleur() {
    QListWidgetItem *item = ui->list_kholleurs->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner un kholleur.");
        return false;
    } else {
        Kholleur* grp = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer le kholleur <strong>\"" + grp->getName() + "\"</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM sec_kholleurs WHERE id=:id");
            query.bindValue(":id", grp->getId());
            query.exec();

            update_list();;
        }
    }

    return true;
}
