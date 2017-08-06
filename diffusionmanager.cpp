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

    for(int i=0; i<selection.length(); i++) {
        Class* cls = (Class*) selection[i]->data(Qt::UserRole).toULongLong();
        if(cls->getOptServer())
            diffuseServer(cls);
    }
}

bool DiffusionManager::diffuseServer(Class* cls) {
    QDate monday = ui->edit_monday->date();

    QString queryDiffuse_str = "";

    QSqlQuery query(QSqlDatabase::database());
    for(int num=1; num<=2; num++) {
        switch(num) {
        case 1:
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs FROM sec_kholles WHERE "
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
            query.prepare("SELECT id, date, time, nb_students, id_kholleurs FROM sec_kholles WHERE "
                          "id_classes = :id_classes AND (date <= '1924-01-01') "
                          "AND id NOT IN "
                          "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_classes", cls->getId());
            query.bindValue(":monday", monday.toString("yyyy-MM-dd"));
            query.exec();
        }

        while (query.next()) {
            QDate date = query.value(1).toDate();
            if(num == 2) {
                int numDays = QDate(1923, 1, 1).daysTo(date);
                date = monday.addDays(numDays);
            }
            QTime time = query.value(2).toTime();
            int nbStudents = query.value(3).toInt();
            int id_kholleurs = query.value(4).toInt();

            queryDiffuse_str += (queryDiffuse_str != "") ? ", " : "";
            queryDiffuse_str += "('"+time.toString("hh:mm:ss")+"', '"+time.addSecs(3600).toString("hh:mm:ss")+"', '"+time.addSecs(0).toString("hh:mm:ss")+"', '";
            queryDiffuse_str += m_kholleurs[id_kholleurs]->getName()+"', '"+date.toString("yyyy-MM-dd")+"', '"+QString::number(nbStudents)+"', '"+cls->getName()+"')";
         }
    }

    if(queryDiffuse_str != "") {
        queryDiffuse_str = "INSERT INTO spark_timeslots(time, time_end, time_start, kholleur, date, nb_pupils, class) VALUES"+queryDiffuse_str+";";
        qDebug() << queryDiffuse_str;
        askODB(queryDiffuse_str, this, test);
    }

    return true;
}

void DiffusionManager::test(ODBRequest *req) {
    QMessageBox::information(this, "Error", req->lastError());
}
