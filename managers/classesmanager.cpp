#include "managers/classesmanager.h"
#include "ui_classesmanager.h"

ClassesManager::ClassesManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClassesManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_class()));
    connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(delete_class()));

    // DB
    m_db = db;
    update_list();
}

ClassesManager::~ClassesManager() {
    delete ui;
    free_classes();
}

bool ClassesManager::free_classes() {
    while (!queue_displayedClasses.isEmpty())
        delete queue_displayedClasses.dequeue();
    return true;
}

bool ClassesManager::update_list() {
    // Clear the list
    ui->list_classes->clear();
    free_classes();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM sec_classes ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Class* cl = new Class();
        cl->setId(query.value(0).toInt());
        cl->setName(query.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(cl->getName(), ui->list_classes);
        item->setData(Qt::UserRole, (qulonglong) cl);
        queue_displayedClasses.enqueue(cl);
    }

    return true;
}

bool ClassesManager::add_class() {
    QString name = ui->lineEdit_name->text();

    if(name == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom de la classe.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO sec_classes(name) VALUES(:name)");
        query.bindValue(":name", name);
        query.exec();

        ui->lineEdit_name->clear();

        update_list();
    }

    return true;
}

bool ClassesManager::delete_class() {
    QListWidgetItem *item = ui->list_classes->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner une classe.");
        return false;
    } else {
        Class* cl = (Class*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer la classe <strong>\"" + cl->getName() + "\"</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM sec_exceptions WHERE id_kholles IN (SELECT id FROM sec_kholles WHERE id_classes = :id_classes);");
            query.bindValue(":id_classes", cl->getId());
            query.exec();
            query.prepare("DELETE FROM sec_kholles WHERE id_classes = :id_classes");
            query.bindValue(":id_classes", cl->getId());
            query.exec();
            query.prepare("DELETE FROM sec_backup_kholles WHERE id_classes = :id_classes");
            query.bindValue(":id_classes", cl->getId());
            query.exec();
            query.prepare("DELETE FROM sec_kholleurs_classes WHERE id_classes = :id_classes");
            query.bindValue(":id_classes", cl->getId());
            query.exec();
            query.prepare("DELETE FROM sec_classes WHERE id=:id");
            query.bindValue(":id", cl->getId());
            query.exec();

            update_list();;
        }
    }

    return true;
}
