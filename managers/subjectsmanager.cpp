#include "managers/subjectsmanager.h"
#include "ui_subjectsmanager.h"

SubjectsManager::SubjectsManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubjectsManager)
{
    // Apply the design of the dialog box
    ui->setupUi(this);
    connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add_subject()));
    connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(delete_subject()));

    // DB
    m_db = db;
    update_list();
}

SubjectsManager::~SubjectsManager() {
    delete ui;
    free_subjects();
}

bool SubjectsManager::free_subjects() {
    while (!queue_displayedSubjects.isEmpty())
        delete queue_displayedSubjects.dequeue();
    return true;
}

bool SubjectsManager::update_list() {
    // Clear the list
    ui->list_subjects->clear();
    free_subjects();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name FROM sec_subjects ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        QListWidgetItem *item = new QListWidgetItem(subj->getName(), ui->list_subjects);
        item->setData(Qt::UserRole, (qulonglong) subj);
        queue_displayedSubjects.enqueue(subj);
    }

    return true;
}

bool SubjectsManager::add_subject() {
    QString name = ui->lineEdit_name->text();

    if(name == "") {
        QMessageBox::critical(this, "Erreur", "Il faut renseigner le nom de la matière.");
        return false;
    } else {
        QSqlQuery query(*m_db);
        query.prepare("INSERT INTO sec_subjects(name) VALUES(:name)");
        query.bindValue(":name", name);
        query.exec();

        ui->lineEdit_name->clear();

        update_list();
    }

    return true;
}

bool SubjectsManager::delete_subject() {
    QListWidgetItem *item = ui->list_subjects->currentItem();

    if(item == NULL) {
        QMessageBox::critical(this, "Erreur", "Veuillez sélectionner une matière.");
        return false;
    } else {
        Subject* subj = (Subject*) item->data(Qt::UserRole).toULongLong();
        int res = QMessageBox::warning(this, "Suppression en cours",
                "Vous êtes sur le point de supprimer la matière <strong>\"" + subj->getName() + "\"</strong>.<br /> Voulez-vous continuer ?",
                QMessageBox::Yes | QMessageBox::Cancel);
        if(res == QMessageBox::Yes) {
            QSqlQuery query(*m_db);
            query.prepare("DELETE FROM sec_subjects WHERE id=:id");
            query.bindValue(":id", subj->getId());
            query.exec();

            update_list();;
        }
    }

    return true;
}
