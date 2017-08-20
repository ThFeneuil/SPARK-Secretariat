#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_list_kholleurs = new QList<Kholleur*>();
    m_list_classes = new QList<Class*>();
    m_list_subjects = new QList<Subject*>();
    m_weekboxes = new QList<WeekBox*>();
    m_weekboxes->append(NULL);
    m_weekboxes->append(NULL);
    m_weekboxes->append(NULL);
    m_firstMonday = QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data.db");
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données...");
    }

    initListsKholleursClassesSubjects();

    connect(ui->action_DB_Classes, SIGNAL(triggered()), this, SLOT(openClassesManager()));
    connect(ui->action_DB_Kholleurs, SIGNAL(triggered()), this, SLOT(openKholleursManager()));
    connect(ui->action_DB_Subjects, SIGNAL(triggered(bool)), this, SLOT(openSubjectsManager()));
    connect(ui->action_Help, SIGNAL(triggered()), this, SLOT(openHelp()));
    connect(ui->action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));
    connect(ui->edit_kholleurs, SIGNAL(textChanged(QString)), this, SLOT(selectKholleur(QString)));
    connect(ui->edit_classes, SIGNAL(textChanged(QString)), this, SLOT(selectClass(QString)));
    connect(ui->list_kholleurs, SIGNAL(currentRowChanged(int)), this, SLOT(kholleurSelected()));
    connect(ui->list_classes, SIGNAL(currentRowChanged(int)), this, SLOT(kholleurSelected()));
    connect(ui->action_Diffusion_options, SIGNAL(triggered(bool)), this, SLOT(openOptions()));
    connect(ui->action_Diffusion_Diffuse, SIGNAL(triggered(bool)), this, SLOT(openDiffusionManager()));
    connect(ui->action_Diffusion_Print, SIGNAL(triggered(bool)), this, SLOT(openPrintDialog()));
    connect(ui->button_publish, SIGNAL(clicked(bool)), this, SLOT(openDiffusionManager()));
    connect(ui->spinBox_preparation, SIGNAL(editingFinished()), this, SLOT(saveDurations()));
    connect(ui->spinBox_kholle, SIGNAL(editingFinished()), this, SLOT(saveDurations()));
    connect(ui->comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(saveDurations()));
    connect(ui->pushButton_all_out, SIGNAL(clicked(bool)), this, SLOT(saveDurationsAll()));

    displayLists();

    if(ui->list_kholleurs->count() > 0)
        ui->list_kholleurs->setCurrentRow(0);
    if(ui->list_classes->count() > 0)
        ui->list_classes->setCurrentRow(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    while(m_list_kholleurs->count() > 0) {
        delete m_list_kholleurs->last();
        m_list_kholleurs->pop_back();
    }
    while(m_list_classes->count() > 0) {
        delete m_list_classes->last();
        m_list_classes->pop_back();
    }
    while(m_list_subjects->count() > 0) {
        delete m_list_subjects->last();
        m_list_subjects->pop_back();
    }
    delete m_list_classes;
    delete m_list_kholleurs;
    delete m_list_subjects;
}

void MainWindow::updateWindow() {
    Kholleur* khllSelected = (ui->list_kholleurs->currentItem()) ? (Kholleur*) ui->list_kholleurs->currentItem()->data(Qt::UserRole).toULongLong() : NULL;
    Class* clSelected = (ui->list_classes->currentItem()) ? ((Class*) ui->list_classes->currentItem()->data(Qt::UserRole).toULongLong()) : NULL;
    int idKholleur = khllSelected ? khllSelected->getId() : 0;
    int idClass = clSelected ? clSelected->getId() : 0;


    initListsKholleursClassesSubjects();
    displayLists();

    for(int i=0; i<ui->list_kholleurs->count() - 1 && idKholleur; i++) {
        Kholleur* khll = (Kholleur*) ui->list_kholleurs->item(i)->data(Qt::UserRole).toULongLong();
        if(khll && khll->getId() == idKholleur)
            ui->list_kholleurs->setCurrentRow(i);
    }
    for(int i=0; i<ui->list_classes->count() - 1 && idClass; i++) {
        Class* cl = (Class*) ui->list_classes->item(i)->data(Qt::UserRole).toULongLong();
        if(cl && cl->getId() == idClass)
            ui->list_classes->setCurrentRow(i);
    }
}
void MainWindow::initListsKholleursClassesSubjects() {
    while(m_list_kholleurs->count() > 0) {
        delete m_list_kholleurs->last();
        m_list_kholleurs->pop_back();
    }
    while(m_list_classes->count() > 0) {
        delete m_list_classes->last();
        m_list_classes->pop_back();
    }
    while(m_list_subjects->count() > 0) {
        delete m_list_subjects->last();
        m_list_subjects->pop_back();
    }

    // Make the request
    QSqlQuery query(QSqlDatabase::database());
    query.exec("SELECT id, name FROM sec_kholleurs ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        m_list_kholleurs->append(khll);
    }

    // Make the request
    query.exec("SELECT id, name FROM sec_classes ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Class* cl = new Class();
        cl->setId(query.value(0).toInt());
        cl->setName(query.value(1).toString());
        m_list_classes->append(cl);
    }

    // Make the request
    query.exec("SELECT id, name FROM sec_subjects ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        m_list_subjects->append(subj);
    }
}

void MainWindow::displayLists() {
    ui->list_kholleurs->clear();
    for(int i=0; i<m_list_kholleurs->count(); i++) {
        Kholleur* khll = m_list_kholleurs->at(i);
        QListWidgetItem *item = new QListWidgetItem(khll->getName(), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
    }
    QListWidgetItem *itemAddKholleur = new QListWidgetItem("Ajouter ce kholleur", ui->list_kholleurs);
    itemAddKholleur->setFont(QFont("", -1, -1, true));
    itemAddKholleur->setData(Qt::UserRole, (qulonglong) NULL);
    itemAddKholleur->setHidden(true);

    ui->list_classes->clear();
    for(int i=0; i<m_list_classes->count(); i++) {
        Class* cl = m_list_classes->at(i);
        QListWidgetItem *item = new QListWidgetItem(cl->getName(), ui->list_classes);
        item->setData(Qt::UserRole, (qulonglong) cl);
    }
    QListWidgetItem *itemAddClass = new QListWidgetItem("Ajouter cette classe", ui->list_classes);
    itemAddClass->setFont(QFont("", -1, -1, true));
    itemAddClass->setData(Qt::UserRole, (qulonglong) NULL);
    itemAddClass->setHidden(true);

    ui->comboBox_subjects->clear();
    ui->comboBox_subjects->addItem("...", (qulonglong) 0);
    for(int i=0; i<m_list_subjects->count(); i++) {
        Subject* subj = m_list_subjects->at(i);
        ui->comboBox_subjects->addItem(subj->getName(), (qulonglong) subj);
    }
}

void MainWindow::openClassesManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        ClassesManager manager(&db, this);
        manager.exec();
        updateWindow();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openKholleursManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        KholleursManager manager(&db, this);
        manager.exec();
        updateWindow();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openSubjectsManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        SubjectsManager manager(&db, this);
        manager.exec();
        updateWindow();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openOptions() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        OptionsDialog manager(&db, this);
        manager.exec();
        updateWindow();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openDiffusionManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        DiffusionManager manager(this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openPrintDialog() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        PrintDialog manager(this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openAboutIt() {
    AboutItDialog dialog(this);
    dialog.exec();
}

void MainWindow::openHelp(){
    ContactDialog dialog(this);
    dialog.exec();
}

void MainWindow::selectKholleur(QString name) {
    selectInList(name, ui->list_kholleurs, TYPE_KHOLLEUR);
}

void MainWindow::selectClass(QString name) {
    selectInList(name, ui->list_classes, TYPE_CLASS);
}

void MainWindow::selectInList(QString name, QListWidget* list, TypeElement type) {
    if(name=="") {
        for(int i=0; i<list->count() - 1; i++)
            list->item(i)->setHidden(false);
    } else {
        int mini = -1;
        for(int i=0; i<list->count() - 1; i++) {
            QListWidgetItem *item = list->item(i);
            if(item->data(Qt::UserRole).toULongLong() == 0)
                continue;
            QString stringItem = "";
            if(type == TYPE_KHOLLEUR)
                    stringItem = ((Kholleur*) item->data(Qt::UserRole).toULongLong())->getName();
            else    stringItem = ((Class*) item->data(Qt::UserRole).toULongLong())->getName();
            int d = distanceLevenshtein(name.toUpper(), stringItem.toUpper(), 2, 0, 2);
            item->setHidden(d < 0);
            if(d>=0 && (mini < 0 || d < mini)) {
                mini = d;
                list->setCurrentItem(item);
            }
        }
        if(mini < 0)
            list->setCurrentRow(list->count()-1);
    }
    list->item(list->count()-1)->setHidden(name=="");
}

void MainWindow::kholleurSelected() {
    displayDurations();
    QListWidgetItem* itemKholleur = ui->list_kholleurs->currentItem();
    QListWidgetItem* itemClass = ui->list_classes->currentItem();
    if(itemKholleur && itemClass) {
        ui->layout_duration->setVisible(true);
        ui->button_addWeek->setVisible(true);
        Kholleur* khll = (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong();
        Class* cl = (Class*) itemClass->data(Qt::UserRole).toULongLong();
        if(khll && cl) {
            ui->title_middleArea->setText("Kholleur : " + khll->getName() + " >>> Classe : " + cl->getName());
            for(int i=0; i<m_weekboxes->count(); i++) {
                if(m_weekboxes->at(i))
                    delete m_weekboxes->at(i);
                if(i==0)
                        m_weekboxes->replace(i, new WeekBox(ALL_MONDAY, khll, cl, ui->spinBox_preparation, ui->spinBox_kholle, 0, m_weekboxes));
                else    m_weekboxes->replace(i, new WeekBox(m_firstMonday.addDays(7*(i-1)), khll, cl, ui->spinBox_preparation, ui->spinBox_kholle, 0));
                ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(i));
            }
        } else
            middleAreaEmpty(khll == NULL, cl == NULL);
    } else
        middleAreaEmpty(itemKholleur == NULL, itemClass == NULL);
}

void MainWindow::middleAreaEmpty(bool noKholleur, bool noClass) {
    ui->layout_duration->setVisible(false);
    ui->button_addWeek->setVisible(false);
    if(noKholleur && noClass)
        ui->title_middleArea->setText("Aucun kholleur sélectionné...<br />Aucune classe sélectionnée...");
    else if(noKholleur)
        ui->title_middleArea->setText("Aucun kholleur sélectionné...");
    else
        ui->title_middleArea->setText("Aucune classe sélectionnée...");

    if(noKholleur || noClass) {
        for(int i=0; i<m_weekboxes->count(); i++) {
            ui->layoutMiddle->removeWidget(m_weekboxes->at(i));
            delete m_weekboxes->at(i);
            m_weekboxes->replace(i, NULL);
        }
    }
}


void MainWindow::addWeek() {
    QSqlDatabase db = QSqlDatabase::database();

    QListWidgetItem* item = ui->list_kholleurs->currentItem();
    if(item) {
        Kholleur* khll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
        Class* cl = (ui->list_classes->currentItem()) ? (Class*) ui->list_classes->currentItem()->data(Qt::UserRole).toULongLong() : NULL;
        if(khll) {
            int n = m_weekboxes->count();
            m_weekboxes->append(new WeekBox(m_firstMonday.addDays(7*n), khll, cl, ui->spinBox_preparation, ui->spinBox_kholle, 0));
            ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(n));
        }
    }
}

int MainWindow::distanceLevenshtein(QString u, QString v, int dmax, int cInsert, int cDelete) {
    int n = u.count();
    int m = v.count();
    int** d = new int*[n+1];
    for(int j=0; j<n+1; j++)
        d[j] = new int[m+1];
    for(int j=0; j<m+1; j++)
        d[0][j] = j;
    for(int i=1; i<n+1; i++)
        d[i][0] = i;
    bool inTheLimit = true;
    for(int i=1; i<=n && inTheLimit; i++) {
        inTheLimit = false;
        for(int j=1; j<=m; j++) {
            int c = (u[i-1] == v[j-1]) ? 0 : 1;
            d[i][j] = d[i][j-1]+cInsert;
            d[i][j] = (d[i][j] < d[i-1][j] + cDelete) ? d[i][j] : d[i-1][j] + 1;
            d[i][j] = (d[i][j] < d[i-1][j-1] + c) ? d[i][j] : d[i-1][j-1] + c;
            if(d[i][j] <= dmax || dmax == -1)
                inTheLimit = true;
        }
    }
    int dL = (inTheLimit && d[n][m] <= dmax) ? d[n][m] : -1;
    for(int j=1; j<n+1; j++)
        delete [] d[1];
    delete [] d;
    return dL;
}

bool MainWindow::eventFilter(QObject* obj, QEvent *event) {
    if (obj == ui->edit_kholleurs || obj == ui->edit_classes) {
        QListWidget* listWidget = (obj == ui->edit_kholleurs) ? ui->list_kholleurs : ui->list_classes;
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Up) {
                int r0 = listWidget->currentRow();
                int r = r0-1;
                while(r >= 0 && r < listWidget->count() && listWidget->item(r)->isHidden())
                    r--;
                listWidget->setCurrentRow((r<0 || r>=listWidget->count()) ? r0 : r);
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Down) {
                int r0 = listWidget->currentRow();
                int r = r0+1;
                while(r >= 0 && r < listWidget->count() && listWidget->item(r)->isHidden())
                    r++;
                listWidget->setCurrentRow((r<0 || r>=listWidget->count()) ? r0 : r);
                return true;
            } else if((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
                      && ((void*) listWidget->currentItem()->data(Qt::UserRole).toULongLong()) == NULL ) {
                QSqlQuery query(QSqlDatabase::database());
                if(obj == ui->edit_kholleurs)
                        query.prepare("INSERT INTO sec_kholleurs(name, duration_preparation, duration_kholle) VALUES(:name, 0, 60)");
                else    query.prepare("INSERT INTO sec_classes(name) VALUES(:name)");
                query.bindValue(":name", ((QLineEdit*) obj)->text());
                query.exec();

                initListsKholleursClassesSubjects();
                displayLists();
                ((QLineEdit*) obj)->clear();
                int idInserted = query.lastInsertId().toInt();
                for(int i=0; i<listWidget->count() - 1; i++) {
                    int id = (obj == ui->edit_kholleurs) ? ((Kholleur*) listWidget->item(i)->data(Qt::UserRole).toULongLong())->getId()
                                                         : ((Class*) listWidget->item(i)->data(Qt::UserRole).toULongLong())->getId();
                    if(id == idInserted)
                        listWidget->setCurrentRow(i);
                }
            }
        }
        return false;
    }
    if(obj == ui->button_addWeek) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton) {
                addWeek();
            }
        }
    }
    return false;
}

void MainWindow::displayDurations() {
    QListWidgetItem* itemKholleur = ui->list_kholleurs->currentItem();
    QListWidgetItem* itemClass = ui->list_classes->currentItem();
    if(itemKholleur && itemClass) {
        Kholleur* khll = (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong();
        Class* cl = (Class*) itemClass->data(Qt::UserRole).toULongLong();
        if(khll && cl) {
            ui->spinBox_preparation->setEnabled(true);
            ui->spinBox_kholle->setEnabled(true);
            QSqlQuery query(QSqlDatabase::database());
            query.prepare("SELECT duration_preparation, duration_kholle, id_subjects FROM sec_kholleurs_classes WHERE id_classes=:id_classes AND id_kholleurs=:id_kholleurs");
            query.bindValue(":id_kholleurs", khll->getId());
            query.bindValue(":id_classes", cl->getId());
            query.exec();
            if(query.next()) {
                ui->spinBox_preparation->setValue(query.value(0).toInt());
                ui->spinBox_kholle->setValue(query.value(1).toInt());
                int id_subjects = query.value(2).toInt();
                ui->comboBox_subjects->setCurrentIndex(0);
                for(int i=0; i<m_list_subjects->count(); i++) {
                    if(m_list_subjects->at(i)->getId() == id_subjects)
                        ui->comboBox_subjects->setCurrentIndex(i+1);
                }
            } else {
                query.prepare("INSERT INTO sec_kholleurs_classes(id_classes, id_kholleurs, duration_preparation, duration_kholle, id_subjects) VALUES(:id_classes, :id_kholleurs, :duration_preparation, :duration_kholle, :id_subjects)");
                query.bindValue(":id_classes", cl->getId());
                query.bindValue(":id_kholleurs", khll->getId());
                query.bindValue(":duration_preparation", 0);
                query.bindValue(":duration_kholle", 60);
                query.bindValue(":id_subjects", 0);
                query.exec();
                ui->spinBox_preparation->setValue(0);
                ui->spinBox_kholle->setValue(60);
                ui->comboBox_subjects->setCurrentIndex(0);
            }
            return;
        }
    }
    ui->spinBox_preparation->setEnabled(false);
    ui->spinBox_kholle->setEnabled(false);

}
void MainWindow::saveDurations() {
    QListWidgetItem* itemKholleur = ui->list_kholleurs->currentItem();
    QListWidgetItem* itemClass = ui->list_classes->currentItem();
    if(itemKholleur && itemClass) {
        Kholleur* khll = (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong();
        Class* cl = (Class*) itemClass->data(Qt::UserRole).toULongLong();
        if(khll && cl) {
            Subject* subj = (Subject*) ui->comboBox_subjects->currentData().toULongLong();
            QSqlQuery query(QSqlDatabase::database());
            query.prepare("UPDATE sec_kholleurs_classes SET duration_preparation=:duration_preparation, duration_kholle=:duration_kholle, id_subjects=:id_subjects WHERE id_kholleurs=:id_kholleurs AND id_classes=:id_classes");
            query.bindValue(":id_kholleurs", khll->getId());
            query.bindValue(":id_classes", cl->getId());
            query.bindValue(":duration_preparation", ui->spinBox_preparation->value());
            query.bindValue(":duration_kholle", ui->spinBox_kholle->value());
            query.bindValue(":id_subjects", (subj) ? subj->getId() : 0);
            query.exec();
        }
    }
}

void MainWindow::saveDurationsAll() {
    ui->pushButton_all_out->setEnabled(false);
    QListWidgetItem* itemKholleur = ui->list_kholleurs->currentItem();
    if(itemKholleur) {
        Kholleur* khll = (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong();
        if(khll) {
            Subject* subj = (Subject*) ui->comboBox_subjects->currentData().toULongLong();
            QSqlQuery query(QSqlDatabase::database());
            for(int i=0; i<m_list_classes->count(); i++) {
                query.prepare("UPDATE sec_kholleurs_classes SET duration_preparation=:duration_preparation, duration_kholle=:duration_kholle, id_subjects=:id_subjects WHERE id_kholleurs=:id_kholleurs AND id_classes=:id_classes");
                query.bindValue(":id_kholleurs", khll->getId());
                query.bindValue(":id_classes", m_list_classes->at(i)->getId());
                query.bindValue(":duration_preparation", ui->spinBox_preparation->value());
                query.bindValue(":duration_kholle", ui->spinBox_kholle->value());
                query.bindValue(":id_subjects", (subj) ? subj->getId() : 0);
                query.exec();

                if(query.numRowsAffected() <= 0) {
                    query.prepare("INSERT INTO sec_kholleurs_classes(id_classes, id_kholleurs, duration_preparation, duration_kholle, id_subjects) VALUES(:id_classes, :id_kholleurs, :duration_preparation, :duration_kholle, :id_subjects)");
                    query.bindValue(":id_classes", m_list_classes->at(i)->getId());
                    query.bindValue(":id_kholleurs", khll->getId());
                    query.bindValue(":duration_preparation", ui->spinBox_preparation->value());
                    query.bindValue(":duration_kholle", ui->spinBox_kholle->value());
                    query.bindValue(":id_subjects", (subj) ? subj->getId() : 0);
                    query.exec();
                }
            }

            QMessageBox::information(this, "Terminé", "Les paramètres de ce kholleurs ont été étendus à toutes les classes...");
        }
    }
    ui->pushButton_all_out->setEnabled(true);
}
