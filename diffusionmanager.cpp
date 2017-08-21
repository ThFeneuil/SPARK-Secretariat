#include "diffusionmanager.h"
#include "ui_diffusionmanager.h"

DiffusionManager::DiffusionManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffusionManager)
{
    ui->setupUi(this);

    QDate next_monday = QDate::currentDate();
    while(next_monday.dayOfWeek() != 1)
        next_monday = next_monday.addDays(1);
    ui->edit_monday->setDate(next_monday);

    QSqlQuery query(QSqlDatabase::database());

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

    QMapIterator<int, Kholleur*> iKholleur(m_kholleurs);
    while (iKholleur.hasNext()) {
        iKholleur.next();
        delete iKholleur.value();
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
    for(int i=-10; i<13; i++) {
        QDate monday = currentMonday.addDays(7*i);
        QListWidgetItem* item = new QListWidgetItem(monday.toString("yyyy-MM-dd"), ui->list_mondays);
        item->setData(Qt::UserRole, monday);
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
    QList<QListWidgetItem*> selection = ui->list_classes->selectedItems();

    ui->label_diffusionHistory->setText("<strong> Initialisation de la diffusion... </strong>");

    QList<Class*> listByPaper;
    QList<Class*> listByServer;

    for(int i=0; i<selection.length(); i++) {
        Class* cls = (Class*) selection[i]->data(Qt::UserRole).toULongLong();
        if(cls->getOptServer())
            listByServer.append(cls);
        if(cls->getOptPaper())
            listByPaper.append(cls);
    }

    // Initialisation
    m_byServer_nbTotal = listByPaper.length();
    m_byServer_nbReceived = 0;
    m_byPaper_built = false;
    m_nbErrors = 0;

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
        PrintPDF::printTimeSlots(ui->edit_monday->date(), listByPaper, QSqlDatabase::database());
        writeDiffusionHistory("Fiches de liaison terminés !");
    } else {
        writeDiffusionHistory("Aucune fiche de liaison à construire...");
    }
    m_byPaper_built = true;

    finishedDiffusion();
}

bool DiffusionManager::diffuseServer(Class* cls) {
    QDate monday = ui->edit_monday->date();

    QString queryDiffuse_str = "";

    QSqlQuery query(QSqlDatabase::database());
    ODBSqlQuery* queryDiffuse = NULL;
    Preferences pref;
    if(pref.serverDefault())
            queryDiffuse = new ODBSqlQuery(DEFAULT INTO(this, requestReturn));
    else    queryDiffuse = new ODBSqlQuery(FROM(pref.serverScript(), pref.serverPassword()) INTO(this, requestReturn));
    for(int num=1; num<=2; num++) {
        switch(num) {
        case 1:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_kholle, id_subjects FROM sec_kholles WHERE "
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
        queryDiffuse->prepare(queryDiffuse_str);
        queryDiffuse->exec();
    } else {
        int num = ++m_byServer_nbReceived;
        writeDiffusionHistory("Classe (par serveur) " + QString::number(num) + "/" + QString::number(m_byServer_nbTotal) + " : Rien à envoyer");
        finishedDiffusion();
    }

    return true;
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
}

void DiffusionManager::finishedDiffusion() {
    if(m_byPaper_built && m_byServer_nbReceived >= m_byServer_nbTotal) {
        writeDiffusionHistory("<strong>DIFFUSION TERMINÉE !</strong>");
        if(m_nbErrors <= 0)
            QMessageBox::information(this, "Diffusion terminé", "La diffusion s'est terminée sans problème.");
        else
            QMessageBox::critical(this, "Diffusion terminé", "La diffusion s'est terminée avec " + QString::number(m_nbErrors) + " " + (m_nbErrors <= 1 ? "erreur" : "erreurs") + " !");
    }
}
