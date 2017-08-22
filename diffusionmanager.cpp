#include "diffusionmanager.h"
#include "ui_diffusionmanager.h"

DiffusionManager::DiffusionManager(QSqlDatabase* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffusionManager)
{
    ui->setupUi(this);
    m_db = db;

    QDate next_monday = QDate::currentDate();
    while(next_monday.dayOfWeek() != 1)
        next_monday = next_monday.addDays(1);
    ui->edit_monday->setDate(next_monday);

    QSqlQuery query(*m_db);

    // Build the map associating an ID with its kholleur
    query.exec("SELECT id, name FROM sec_kholleurs");
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        m_kholleurs.insert(khll->getId(), khll);
    }

    // Build the map associating an ID with its subjects
    query.exec("SELECT id, name FROM sec_subjects");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        m_subjects.insert(subj->getId(), subj);
    }

    // Build the map associating an ID with its class
    query.exec("SELECT id, name, by_server, by_email, by_paper, email FROM sec_classes ORDER BY UPPER(name)");
    while (query.next()) {
        Class* cl = new Class();
        cl->setId(query.value(0).toInt());
        cl->setName(query.value(1).toString());
        cl->setOptServer(query.value(2).toBool());
        cl->setOptEmail(query.value(3).toBool());
        cl->setOptPaper(query.value(4).toBool());
        cl->setEmail(query.value(5).toString());
        QListWidgetItem* item = new QListWidgetItem(cl->getName(), ui->list_classes);
        item->setData(Qt::UserRole, (qulonglong) cl);
        m_classes.insert(cl->getId(), cl);
    }

    connect(ui->button_valid, SIGNAL(clicked(bool)), this, SLOT(diffuse()));
    connect(ui->list_classes, SIGNAL(itemSelectionChanged()), this, SLOT(infoLabel()));

    ui->list_classes->selectAll();
    infoLabel();
    update_list_mondays();
    connect(ui->edit_monday, SIGNAL(dateChanged(QDate)), this, SLOT(update_list_mondays()));
    connect(ui->list_mondays, SIGNAL(itemSelectionChanged()), this, SLOT(update_edit_monday()));
}

DiffusionManager::~DiffusionManager()
{
    delete ui;

    QMapIterator<int, Kholleur*> iKholleurs(m_kholleurs);
    while (iKholleurs.hasNext()) {
        iKholleurs.next();
        delete iKholleurs.value();
    }
    QMapIterator<int, Subject*> iSubjects(m_subjects);
    while (iSubjects.hasNext()) {
        iSubjects.next();
        delete iSubjects.value();
    }
    QMapIterator<int, Class*> iClasses(m_classes);
    while (iClasses.hasNext()) {
        iClasses.next();
        delete iClasses.value();
    }
}

void DiffusionManager::update_list_mondays() {
    QDate currentMonday = ui->edit_monday->date();

    if(currentMonday.dayOfWeek() != 1) {
        QMessageBox::information(this, "Correction de la date", "La date renseignée n'est pas un Lundi. Le lundi sélectionné sera donc le lundi de la même semaine que le jour rentré.");
        currentMonday = currentMonday.addDays(1-currentMonday.dayOfWeek());
    }

    ui->list_mondays->clear();
    QSqlQuery query(*m_db);
    for(int i=-10; i<13; i++) {
        QDate monday = currentMonday.addDays(7*i);
        QListWidgetItem* item = new QListWidgetItem(monday.toString("yyyy-MM-dd"), ui->list_mondays);
        item->setData(Qt::UserRole, monday);

        query.prepare("SELECT COUNT(*) FROM sec_backup_kholles WHERE date >= :start AND date <= :end");
        query.bindValue(":start", monday.toString("yyyy-MM-dd"));
        query.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
        query.exec();
        if(query.next() &&  query.value(0).toInt() > 0)
            item->setBackgroundColor(QColor(Qt::green));
    }
    ui->list_mondays->setCurrentRow(10);
}

void DiffusionManager::update_edit_monday() {
    int row = ui->list_mondays->currentRow();
    QListWidgetItem* item = ui->list_mondays->item(row);
    if(item) {
        QDate currentMonday = item->data(Qt::UserRole).toDate();
        disconnect(ui->edit_monday, SIGNAL(dateChanged(QDate)), this, SLOT(update_list_mondays()));
        ui->edit_monday->setDate(currentMonday);
        connect(ui->edit_monday, SIGNAL(dateChanged(QDate)), this, SLOT(update_list_mondays()));
        if(row < 3 || row > 19)
            update_list_mondays();
    }
}

void DiffusionManager::infoLabel() {
    QList<QListWidgetItem*> selection = ui->list_classes->selectedItems();

    int by_server = 0;
    int by_email = 0;
    int by_paper = 0;

    for(int i=0; i<selection.length(); i++) {
        Class* cls = (Class*) selection[i]->data(Qt::UserRole).toULongLong();
        by_server += cls->getOptServer() ? 1 : 0;
        by_email += cls->getOptEmail() ? 1 : 0;
        by_paper += cls->getOptPaper() ? 1 : 0;
    }

    ui->label_info->setText("Diffusion par voie papier : " + QString::number(by_paper) + "<br />" +
                            "Diffusion par email : " + QString::number(by_email) + "<br />" +
                            "Diffusion par serveur : " + QString::number(by_server));
}

void DiffusionManager::diffuse() {
    ui->button_valid->setEnabled(false);
    QList<QListWidgetItem*> selection = ui->list_classes->selectedItems();
    QDate monday = ui->edit_monday->date();

    ui->label_diffusionHistory->setText("<strong> Initialisation de la diffusion... </strong>");

    QList<Class*> listByPaper;
    QList<Class*> listByServer;
    QList<Class*> listAlreadyInBackup;

    QSqlQuery query(*m_db);
    for(int i=0; i<selection.length(); i++) {
        Class* cls = (Class*) selection[i]->data(Qt::UserRole).toULongLong();
        if(cls->getOptServer())
            listByServer.append(cls);
        if(cls->getOptPaper())
            listByPaper.append(cls);

        query.prepare("SELECT COUNT(*) FROM sec_backup_kholles WHERE "
                      "id_classes = :id_classes AND (date >= :start AND date <= :end) ");
        query.bindValue(":id_classes", cls->getId());
        query.bindValue(":start", monday.toString("yyyy-MM-dd"));
        query.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
        query.exec();
        if(query.next()) {
            int nb = query.value(0).toInt();
            if(nb > 0)
                listAlreadyInBackup.append(cls);
        }
    }

    // Initialisation
    m_byServer_nbTotal = listByServer.length();
    m_byServer_nbReceived = 0;
    m_byPaper_built = false;
    m_diffuseInBackup = false;
    m_nbErrors = 0;
    m_replaceTimeslots = true;

    if(listAlreadyInBackup.count() > 0) {
        QMessageBox msg;
        msg.setWindowTitle("Diffusion");
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Parmi les classes sélectionnées, certaines ont déjà été diffusées... Voulez-vous remplacer la diffusion précédente ou voulez-vous rajouter cette diffusion à la précédente ? <strong>Attention, si vous la rajoutez, certains horaires de kholles pourront être dédoublés !</strong>");
        QAbstractButton *replace_btn = (QAbstractButton*) msg.addButton("Remplacer", QMessageBox::ApplyRole);
        QAbstractButton *add_btn = (QAbstractButton*) msg.addButton("Rajouter (!)", QMessageBox::ApplyRole);
        msg.addButton("Annuler", QMessageBox::ApplyRole);
        msg.exec();

        if(msg.clickedButton() == replace_btn) {
            m_replaceTimeslots = true;
        } else if(msg.clickedButton() == add_btn) {
            m_replaceTimeslots = false;
        } else {
            writeDiffusionHistory("<strong> DIFFUSION ANNULÉE ! </strong>");
            ui->button_valid->setEnabled(true);
            return;
        }
    }

    ui->label_diffusionHistory->setText("<strong> Démarrage de la diffusion... </strong>");

    if(m_byServer_nbTotal > 0) {
        writeDiffusionHistory("Démarrage de l'envoi sur le serveur.");
        writeDiffusionHistory("Il y a "+QString::number(m_byServer_nbTotal) + " " + (m_byServer_nbTotal <= 1 ? "classe" : "classes") + " à envoyer.");
        for(int i=0; i<listByServer.length(); i++)
            diffuseServer(listByServer[i]);
    } else {
        writeDiffusionHistory("Aucune classe à envoyer sur le serveur...");
    }

    if(listByPaper.length() > 0) {
        writeDiffusionHistory("Construction des fiches de liaison.");
        PrintPDF::printTimeSlots(ui->edit_monday->date(), listByPaper, *m_db, this);
        writeDiffusionHistory("Fiches de liaison terminés !");
    } else {
        writeDiffusionHistory("Aucune fiche de liaison à construire.");
    }
    m_byPaper_built = true;

    writeDiffusionHistory("Sauvegarde des horaires de kholles.");
    int nbTimeSlots = 0;
    for(int i=0; i<selection.length(); i++)
        nbTimeSlots += diffuseInBackup((Class*) selection[i]->data(Qt::UserRole).toULongLong());
    writeDiffusionHistory("Nombre d'horaires de kholles sauvegardés: " + QString::number(nbTimeSlots));
    writeDiffusionHistory("Sauvegarde terminée.");
    m_diffuseInBackup = true;

    finishedDiffusion();
}

bool DiffusionManager::diffuseServer(Class* cls) {
    QDate monday = ui->edit_monday->date();

    QString queryDiffuse_str = "";

    QSqlQuery query(*m_db);
    ODBSqlQuery* queryDiffuse = NULL;
    Preferences pref;
    if(pref.serverDefault())
            queryDiffuse = new ODBSqlQuery(DEFAULT INTO(this, requestReturn));
    else    queryDiffuse = new ODBSqlQuery(FROM(pref.serverScript(), pref.serverPassword()) INTO(this, requestReturn));
    for(int num=1; num<=2; num++) {
        switch(num) {
        case 1:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                          "id_classes = :id_classes AND (date >= :start AND date <= :end) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_classes", cls->getId());
            query.bindValue(":start", monday.toString("yyyy-MM-dd"));
            query.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
            query.exec();
            break;
        case 2:
        default:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                          "id_classes = :id_classes AND (date <= '1924-01-01') "
                          "AND id NOT IN "
                          "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_classes", cls->getId());
            query.bindValue(":monday", monday.toString("yyyy-MM-dd"));
            query.exec();
        }

        for(int numRow = 1; query.next(); numRow++) {
            QDate date = query.value(1).toDate();
            if(num == 2) {
                int numDays = QDate(1923, 1, 1).daysTo(date);
                date = monday.addDays(numDays);
            }
            QTime time = query.value(2).toTime();
            int nbStudents = query.value(3).toInt();
            int id_kholleurs = query.value(4).toInt();
            int duration_preparation = query.value(5).toInt();
            int duration_kholle = query.value(6).toInt();
            int id_subjects = query.value(7).toInt();

            QString strRow = QString::number(numRow);

            queryDiffuse_str += (queryDiffuse_str != "") ? ", " : "";
            queryDiffuse_str += "(:time"+strRow+", :time_end"+strRow+", :time_start"+strRow+", ";
            queryDiffuse_str += ":nameKholleur"+strRow+", :date"+strRow+", :nbStudents"+strRow+", :nameClass"+strRow+", :subject"+strRow+")";

            queryDiffuse->bindValue(":time"+strRow, time.addSecs(duration_preparation*60).toString("hh:mm:ss"));
            queryDiffuse->bindValue(":time_end"+strRow, time.addSecs((duration_preparation+duration_kholle)*60).toString("hh:mm:ss"));
            queryDiffuse->bindValue(":time_start"+strRow, time.toString("hh:mm:ss"));
            queryDiffuse->bindValue(":nameKholleur"+strRow, m_kholleurs[id_kholleurs]->getName());
            queryDiffuse->bindValue(":date"+strRow, date.toString("yyyy-MM-dd"));
            queryDiffuse->bindValue(":nbStudents"+strRow, QString::number(nbStudents));
            queryDiffuse->bindValue(":nameClass"+strRow, cls->getName());
            queryDiffuse->bindValue(":subject"+strRow, (id_subjects ? m_subjects[id_subjects]->getName() : "..."));
         }
    }

    if(queryDiffuse_str != "") {
        queryDiffuse_str = "INSERT INTO spark_timeslots(time, time_end, time_start, kholleur, date, nb_pupils, class, subject) VALUES"+queryDiffuse_str+";";
        if(m_replaceTimeslots) {
            queryDiffuse_str = "DELETE FROM spark_timeslots WHERE class = :nameClass AND (date >= :start AND date <= :end); "+queryDiffuse_str;
            queryDiffuse->bindValue(":nameClass", cls->getName());
            queryDiffuse->bindValue(":start", monday.toString("yyyy-MM-dd"));
            queryDiffuse->bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
        }
        queryDiffuse->prepare(queryDiffuse_str);
        queryDiffuse->exec();
    } else {
        int num = ++m_byServer_nbReceived;
        writeDiffusionHistory("Classe (par serveur) " + QString::number(num) + "/" + QString::number(m_byServer_nbTotal) + " : Rien à envoyer");
        finishedDiffusion();
    }

    return true;
}

int DiffusionManager::diffuseInBackup(Class* cls) {
    QDate monday = ui->edit_monday->date();

    QString queryBackup_str = "";

    QSqlQuery query(*m_db);
    QSqlQuery queryBackup(*m_db);
    QMap<QString, QVariant> bindValues;
    int nbTimeSlots = 0;
    for(int num=1; num<=2; num++) {
        switch(num) {
        case 1:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                          "id_classes = :id_classes AND (date >= :start AND date <= :end) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_classes", cls->getId());
            query.bindValue(":start", monday.toString("yyyy-MM-dd"));
            query.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
            query.exec();
            break;
        case 2:
        default:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                          "id_classes = :id_classes AND (date <= '1924-01-01') "
                          "AND id NOT IN "
                          "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_classes", cls->getId());
            query.bindValue(":monday", monday.toString("yyyy-MM-dd"));
            query.exec();
        }

        for(int numRow = 1; query.next(); numRow++) {
            QDate date = query.value(1).toDate();
            if(num == 2) {
                int numDays = QDate(1923, 1, 1).daysTo(date);
                date = monday.addDays(numDays);
            }
            QTime time = query.value(2).toTime();
            int nbStudents = query.value(3).toInt();
            int id_kholleurs = query.value(4).toInt();
            int duration_preparation = query.value(5).toInt();
            int duration_kholle = query.value(6).toInt();
            int id_subjects = query.value(7).toInt();

            QString strRow = QString::number(numRow);

            queryBackup_str += (queryBackup_str != "") ? ", " : "";
            queryBackup_str += "(:id_kholleurs"+strRow+", :id_classes"+strRow+", :date"+strRow+", :time"+strRow+", :nb_students"+strRow+", ";
            queryBackup_str += ":duration_preparation"+strRow+", :duration_kholle"+strRow+", :id_subjects"+strRow+")";

            bindValues.insert(":id_kholleurs"+strRow, id_kholleurs);
            bindValues.insert(":id_classes"+strRow, cls->getId());
            bindValues.insert(":date"+strRow, date.toString("yyyy-MM-dd"));
            bindValues.insert(":time"+strRow, time.toString("hh:mm:ss"));
            bindValues.insert(":nb_students"+strRow, nbStudents);
            bindValues.insert(":duration_preparation"+strRow, duration_preparation);
            bindValues.insert(":duration_kholle"+strRow, duration_kholle);
            bindValues.insert(":id_subjects"+strRow, id_subjects);

            nbTimeSlots++;
         }
    }

    if(queryBackup_str != "") {
        queryBackup_str = "INSERT INTO sec_backup_kholles(id_kholleurs, id_classes, date, time, nb_students, duration_preparation, duration_kholle, id_subjects) VALUES"+queryBackup_str;
        if(m_replaceTimeslots) {
            queryBackup.prepare("DELETE FROM sec_backup_kholles WHERE id_classes = :id_classes AND (date >= :start AND date <= :end);");
            queryBackup.bindValue(":id_classes", cls->getId());
            queryBackup.bindValue(":start", monday.toString("yyyy-MM-dd"));
            queryBackup.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
            queryBackup.exec();
        }
        queryBackup.prepare(queryBackup_str);
        QMapIterator<QString, QVariant> iBindValues(bindValues);
        while(iBindValues.hasNext()) {
            iBindValues.next();
            queryBackup.bindValue(iBindValues.key(), iBindValues.value());
        }
        queryBackup.exec();
    }

    return nbTimeSlots;
}

void DiffusionManager::requestReturn(ODBRequest *req) {
    int num = ++m_byServer_nbReceived;
    if(req->lastError() == "") {
        writeDiffusionHistory("Classe (par serveur) " + QString::number(num) + "/" + QString::number(m_byServer_nbTotal) + " : Envoyé");
    } else {
        writeDiffusionHistory("<strong style='color:red'>" + req->lastError() + "</strong>");
        writeDiffusionHistory("SERVER ERROR : Classe " + QString::number(num) + "/" + QString::number(m_byServer_nbTotal));
        m_nbErrors++;
    }
    finishedDiffusion();
}

void DiffusionManager::writeDiffusionHistory(QString text) {
    ui->label_diffusionHistory->setText(text + "<br />" + ui->label_diffusionHistory->text());
    show();
}

void DiffusionManager::finishedDiffusion() {
    if(m_byPaper_built && m_diffuseInBackup && m_byServer_nbReceived >= m_byServer_nbTotal) {
        update_list_mondays();
        writeDiffusionHistory("<strong>DIFFUSION TERMINÉE !</strong>");
        ui->button_valid->setEnabled(true);
        if(m_nbErrors <= 0)
            QMessageBox::information(this, "Diffusion terminé", "La diffusion s'est terminée sans problème.");
        else
            QMessageBox::critical(this, "Diffusion terminé", "La diffusion s'est terminée avec " + QString::number(m_nbErrors) + " " + (m_nbErrors <= 1 ? "erreur" : "erreurs") + " !");
    }
}
