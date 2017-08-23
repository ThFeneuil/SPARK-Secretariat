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
    m_firstMonday = QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek());
    m_saveIdSelecKholleur = 0;
    m_saveIdSelecClass = 0;
    m_researchTimer = new QTimer(this);
    m_researchTimer->setSingleShot(true);

    m_weekboxes = new QList<WeekBox*>();
    m_weekboxes->append(new WeekBox(ALL_MONDAY, NULL, NULL, ui->spinBox_preparation, ui->spinBox_kholle, ui->comboBox_subjects, m_list_subjects, 0, m_weekboxes));
    m_weekboxes->append(new WeekBox(m_firstMonday, NULL, NULL, ui->spinBox_preparation, ui->spinBox_kholle, ui->comboBox_subjects, m_list_subjects, 0));
    m_weekboxes->append(new WeekBox(m_firstMonday.addDays(7), NULL, NULL, ui->spinBox_preparation, ui->spinBox_kholle, ui->comboBox_subjects, m_list_subjects, 0));
    ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(0));
    ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(1));
    ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(2));

    connect(ui->action_File_Create, SIGNAL(triggered(bool)), this, SLOT(createSEC()));
    connect(ui->action_File_Open, SIGNAL(triggered(bool)), this, SLOT(openSEC()));
    connect(ui->action_File_Settings, SIGNAL(triggered(bool)), this, SLOT(openSettings()));
    connect(ui->action_File_Quit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->action_DB_Classes, SIGNAL(triggered()), this, SLOT(openClassesManager()));
    connect(ui->action_DB_Kholleurs, SIGNAL(triggered()), this, SLOT(openKholleursManager()));
    connect(ui->action_DB_Subjects, SIGNAL(triggered(bool)), this, SLOT(openSubjectsManager()));
    connect(ui->action_Diffusion_options, SIGNAL(triggered(bool)), this, SLOT(openOptions()));
    connect(ui->action_Diffusion_Diffuse, SIGNAL(triggered(bool)), this, SLOT(openDiffusionManager()));
    connect(ui->action_Diffusion_KhollesPaper, SIGNAL(triggered(bool)), this, SLOT(openKhollesPaperManager()));

    //Pour le A propos de...
    QAction *action_AboutIt = new QAction("A propos de...");
    ui->menubar->addAction(action_AboutIt);
    connect(action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));

    connect(ui->edit_kholleurs, SIGNAL(textChanged(QString)), this, SLOT(selectKholleur(QString)));
    connect(ui->edit_classes, SIGNAL(textChanged(QString)), this, SLOT(selectClass(QString)));
    connect(ui->list_kholleurs, SIGNAL(currentRowChanged(int)), this, SLOT(kholleurSelected()));
    connect(ui->list_classes, SIGNAL(currentRowChanged(int)), this, SLOT(kholleurSelected()));
    connect(ui->button_publish, SIGNAL(clicked(bool)), this, SLOT(openDiffusionManager()));
    connect(ui->spinBox_preparation, SIGNAL(editingFinished()), this, SLOT(saveDurations()));
    connect(ui->spinBox_kholle, SIGNAL(editingFinished()), this, SLOT(saveDurations()));
    connect(ui->comboBox_subjects, SIGNAL(currentIndexChanged(int)), this, SLOT(saveDurations()));
    connect(ui->pushButton_all_out, SIGNAL(clicked(bool)), this, SLOT(saveDurationsAll()));
    connect(m_researchTimer, SIGNAL(timeout()), this, SLOT(selectInList()));

    openSEC(true);

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
    QSqlDatabase db = QSqlDatabase::database();

    if(db.databaseName() == "")
        db.close();

    ui->action_DB_Classes->setEnabled(db.isOpen());
    ui->action_DB_Kholleurs->setEnabled(db.isOpen());
    ui->action_DB_Subjects->setEnabled(db.isOpen());
    ui->action_Diffusion_options->setEnabled(db.isOpen());
    ui->action_Diffusion_Diffuse->setEnabled(db.isOpen());
    ui->action_Diffusion_KhollesPaper->setEnabled(db.isOpen());
    ui->list_kholleurs->setEnabled(db.isOpen());
    ui->list_classes->setEnabled(db.isOpen());
    ui->edit_kholleurs->setEnabled(db.isOpen());
    ui->edit_classes->setEnabled(db.isOpen());
    ui->button_publish->setEnabled(db.isOpen());

    ui->layout_duration->setVisible(false);
    ui->button_addWeek->setVisible(false);

    if(! db.isOpen())
        return;

    saveSelection();
    initListsKholleursClassesSubjects();
    displayLists();
    putSavedSelection();

    kholleurSelected();
}

void MainWindow::initListsKholleursClassesSubjects() {
    initListKholleurs();
    initListClasses();
    initListSubjects();

    loadParametersKholleursClasses();
}

void MainWindow::initListKholleurs() {
    while(m_list_kholleurs->count() > 0) {
        delete m_list_kholleurs->last();
        m_list_kholleurs->pop_back();
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
}

void MainWindow::initListClasses() {
    while(m_list_classes->count() > 0) {
        delete m_list_classes->last();
        m_list_classes->pop_back();
    }

    // Make the request
    QSqlQuery query(QSqlDatabase::database());
    query.exec("SELECT id, name FROM sec_classes ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Class* cl = new Class();
        cl->setId(query.value(0).toInt());
        cl->setName(query.value(1).toString());
        m_list_classes->append(cl);
    }
}

void MainWindow::initListSubjects() {
    while(m_list_subjects->count() > 0) {
        delete m_list_subjects->last();
        m_list_subjects->pop_back();
    }

    // Make the request
    QSqlQuery query(QSqlDatabase::database());
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
        DiffusionManager manager(&db, this);
        manager.exec();
    }
    else {
        QMessageBox::critical(this, "Erreur", "La connexion à la base de données a échoué");
    }
}

void MainWindow::openKhollesPaperManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        KhollesPaperManager manager(&db, this);
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

void MainWindow::selectKholleur(QString name) {
    waitAndSelectInList(name, ui->list_kholleurs, TYPE_KHOLLEUR);
}

void MainWindow::selectClass(QString name) {
    waitAndSelectInList(name, ui->list_classes, TYPE_CLASS);
}

void MainWindow::waitAndSelectInList(QString name, QListWidget* list, TypeElement type) {
    m_researchTimer->stop();
    m_researchName = name;
    m_researchList = list;
    m_researchType = type;
    m_researchTimer->start(150);
}

void MainWindow::selectInList() {
    QString name = m_researchName;
    QListWidget* list = m_researchList;
    TypeElement type = m_researchType;

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
    Kholleur* khll = itemKholleur ? (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong() : NULL;
    Class* cl = itemClass ? (Class*) itemClass->data(Qt::UserRole).toULongLong() : NULL;

    ui->layout_duration->setVisible(khll && cl);
    ui->button_addWeek->setVisible(khll && cl);

    ui->title_middleArea->setText("");

    if(khll && cl)
        ui->title_middleArea->setText("Kholleur : " + khll->getName() + " >>> Classe : " + cl->getName());
    if(khll == NULL)
        ui->title_middleArea->setText("Aucun kholleur sélectionné...<br />");
    if(cl == NULL)
        ui->title_middleArea->setText(ui->title_middleArea->text() + "Aucun classe sélectionnée...");

    for(int i=0; i<m_weekboxes->count(); i++)
        m_weekboxes->at(i)->setKholleurClass(khll, cl);
}

void MainWindow::addWeek() {
    QSqlDatabase db = QSqlDatabase::database();

    QListWidgetItem* itemKholleur = ui->list_kholleurs->currentItem();
    QListWidgetItem* itemClass = ui->list_classes->currentItem();
    Kholleur* khll = itemKholleur ? (Kholleur*) itemKholleur->data(Qt::UserRole).toULongLong() : NULL;
    Class* cl = itemClass ? (Class*) itemClass->data(Qt::UserRole).toULongLong() : NULL;

    int n = m_weekboxes->count();
    m_weekboxes->append(new WeekBox(m_firstMonday.addDays(7*n), NULL, NULL, ui->spinBox_preparation, ui->spinBox_kholle, ui->comboBox_subjects, m_list_subjects, 0));
    ui->layoutMiddle->insertWidget(ui->layoutMiddle->count()-1, m_weekboxes->at(n));
    m_weekboxes->at(n)->setKholleurClass(khll, cl);
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
                        query.prepare("INSERT INTO sec_kholleurs(name) VALUES(:name)");
                else    query.prepare("INSERT INTO sec_classes(name) VALUES(:name)");
                query.bindValue(":name", ((QLineEdit*) obj)->text());
                query.exec();
                int idInserted = query.lastInsertId().toInt();

                updateWindow();
                ((QLineEdit*) obj)->clear();
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

            ui->spinBox_preparation->setValue(m_paraKC[khll->getId()][cl->getId()].duration_preparation);
            ui->spinBox_kholle->setValue(m_paraKC[khll->getId()][cl->getId()].duration_kholle);
            int id_subjects = m_paraKC[khll->getId()][cl->getId()].id_subjects;
            ui->comboBox_subjects->setCurrentIndex(0);
            for(int i=0; i<m_list_subjects->count(); i++) {
                if(m_list_subjects->at(i)->getId() == id_subjects)
                    ui->comboBox_subjects->setCurrentIndex(i+1);
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
            m_paraKC[khll->getId()][cl->getId()].duration_preparation = ui->spinBox_preparation->value();
            m_paraKC[khll->getId()][cl->getId()].duration_kholle = ui->spinBox_kholle->value();
            m_paraKC[khll->getId()][cl->getId()].id_subjects = (subj) ? subj->getId() : 0;
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
            query.prepare("UPDATE sec_kholleurs_classes SET duration_preparation=:duration_preparation, duration_kholle=:duration_kholle, id_subjects=:id_subjects WHERE id_kholleurs=:id_kholleurs");
            query.bindValue(":id_kholleurs", khll->getId());
            query.bindValue(":duration_preparation", ui->spinBox_preparation->value());
            query.bindValue(":duration_kholle", ui->spinBox_kholle->value());
            query.bindValue(":id_subjects", (subj) ? subj->getId() : 0);
            query.exec();

            for(int i=0; i<m_list_classes->count(); i++) {
                Class* cl = m_list_classes->at(i);
                m_paraKC[khll->getId()][cl->getId()].duration_preparation = ui->spinBox_preparation->value();
                m_paraKC[khll->getId()][cl->getId()].duration_kholle = ui->spinBox_kholle->value();
                m_paraKC[khll->getId()][cl->getId()].id_subjects = (subj) ? subj->getId() : 0;
            }

            QMessageBox::information(this, "Terminé", "Les paramètres de ce kholleurs ont été étendus à toutes les classes...");
        }
    }
    ui->pushButton_all_out->setEnabled(true);
}

void MainWindow::loadParametersKholleursClasses() {
    m_paraKC.clear();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.exec("SELECT id_classes, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholleurs_classes");

    while(query.next()) {
        int id_classes = query.value(0).toInt();
        int id_kholleurs = query.value(1).toInt();
        ParametersKholleurClass par;
        par.duration_preparation = query.value(2).toInt();
        par.duration_kholle = query.value(3).toInt();
        par.id_subjects = query.value(4).toInt();

        m_paraKC[id_kholleurs][id_classes] = par;
    }

    db.transaction();
    for(int i=0; i<m_list_kholleurs->count(); i++) {
        for(int j=0; j<m_list_classes->count(); j++) {
            Kholleur* khll = m_list_kholleurs->at(i);
            Class* cl = m_list_classes->at(j);
            if(! (m_paraKC.contains(khll->getId()) && m_paraKC[khll->getId()].contains(cl->getId()))) {
                query.prepare("INSERT INTO sec_kholleurs_classes(id_classes, id_kholleurs, duration_preparation, duration_kholle, id_subjects) VALUES(:id_classes, :id_kholleurs, :duration_preparation, :duration_kholle, :id_subjects)");
                query.bindValue(":id_classes", cl->getId());
                query.bindValue(":id_kholleurs", khll->getId());
                query.bindValue(":duration_preparation", 0);
                query.bindValue(":duration_kholle", 60);
                query.bindValue(":id_subjects", 0);
                query.exec();

                ParametersKholleurClass par;
                par.duration_preparation = 0;
                par.duration_kholle = 60;
                par.id_subjects = 0;
                m_paraKC[khll->getId()][cl->getId()] = par;
            }
        }
    }
    db.commit();
}

void MainWindow::openSEC(bool withPref) {
    Preferences pref;
    QString filename = "";
    bool ok = false;
    if(withPref) {
        filename = pref.file();
        if(filename != "") {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(filename);
            ok = db.open();
        } else {
            openSEC(false);
            return;
        }
    } else {
        filename = QFileDialog::getOpenFileName(NULL, "Ouvrir...", pref.dir(),  "SEC (*.sec)");
        if(QFile::exists(filename))
            pref.setFile(filename);
        if(filename != "") {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            pref.setDir(QFileInfo(filename).absoluteDir().absolutePath());
            db.setDatabaseName(filename);
            ok = db.open();
        } else {
            updateWindow();
            return;
        }
    }

    QString textFile = (filename.count()>90) ? "..." + filename.right(90) : filename;
    if(ok) {
        ui->label_general_info->setText("Fichier : "+textFile+" >>> <strong>Chargé !</strong>");
    } else {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données...");
        ui->label_general_info->setText("Fichier : "+textFile+" >>> <strong>ECHEC !</strong>");
    }
    updateWindow();
}

void MainWindow::createSEC() {
    //Try to load directory preferences
    Preferences pref;
    QString pref_path = pref.dir();

    //Get file name
    QString filename = QFileDialog::getSaveFileName(this, "Enregistrer sous...",
                                                    pref_path + QDir::separator() + "horaires-kholles",  "SEC (*.sec)");

    if(filename == "") {
        updateWindow();
        return;
    }

    //Save directory in preferences
    QString dirpath = QFileInfo(filename).absoluteDir().absolutePath();
    pref.setDir(dirpath);
    pref.setFile(filename);
    QString textFile = (filename.count()>90) ? "..." + filename.right(90) : filename;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    if (!db.open()) {
        updateWindow();
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données générée...");
        ui->label_general_info->setText("Fichier : "+textFile+" >>> <strong>ECHEC !</strong>");
        return;
    } else {
        ui->label_general_info->setText("Fichier : "+textFile+" >>> <strong>Chargé !</strong>");
    }

    QSqlQuery qCreate(db);
    qCreate.exec("CREATE TABLE `sec_backup_kholles` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                     "`id_kholleurs`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_classes`	INTEGER NOT NULL DEFAULT 0, "
                     "`date`	TEXT NOT NULL DEFAULT '', "
                     "`time`	TEXT NOT NULL DEFAULT '', "
                     "`nb_students`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_preparation`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_kholle`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_subjects`	INTEGER NOT NULL DEFAULT 0 "
                 ");");
    qCreate.exec("CREATE TABLE `sec_classes` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`name`	TEXT NOT NULL, "
                     "`by_server`	INTEGER NOT NULL DEFAULT 0, "
                     "`by_email`	INTEGER NOT NULL DEFAULT 0, "
                     "`by_paper`	INTEGER NOT NULL DEFAULT 0, "
                     "`email`	TEXT NOT NULL DEFAULT '' "
                 ");");
    qCreate.exec("CREATE TABLE `sec_exceptions` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`id_kholles`	INTEGER NOT NULL, "
                     "`monday`	TEXT NOT NULL "
                 ");");
    qCreate.exec("CREATE TABLE `sec_kholles` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`id_kholleurs`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_classes`	INTEGER NOT NULL DEFAULT 0, "
                     "`date`	TEXT NOT NULL DEFAULT '', "
                     "`time`	TEXT NOT NULL DEFAULT '', "
                     "`nb_students`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_preparation`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_kholle`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_subjects`	INTEGER NOT NULL DEFAULT 0 "
                  ");");
    qCreate.exec("CREATE TABLE `sec_kholleurs` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`name`	TEXT NOT NULL DEFAULT '' "
                ");");
    qCreate.exec("CREATE TABLE `sec_kholleurs_classes` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`id_classes`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_kholleurs`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_preparation`	INTEGER NOT NULL DEFAULT 0, "
                     "`duration_kholle`	INTEGER NOT NULL DEFAULT 0, "
                     "`id_subjects`	INTEGER NOT NULL DEFAULT 0 "
                 ");");
    qCreate.exec("CREATE TABLE `sec_subjects` ( "
                     "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                     "`name`	TEXT NOT NULL DEFAULT '' "
                 ");");

    QMessageBox::information(NULL, "Succès", "Votre base de données a été créée.<br />Vous pouvez dès maintenant l'utiliser. :p");
    updateWindow();
}

void MainWindow::openSettings() {
    // Open the dialog
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::saveSelection() {
    QListWidgetItem* itemSelec = NULL;

    itemSelec = ui->list_kholleurs->currentItem();
    if(itemSelec)
            m_saveIdSelecKholleur =  itemSelec->data(Qt::UserRole).toULongLong() ? ((Kholleur*) itemSelec->data(Qt::UserRole).toULongLong())->getId() : 0;
    else    m_saveIdSelecKholleur = 0;

    itemSelec = ui->list_classes->currentItem();
    if(itemSelec)
            m_saveIdSelecClass =  itemSelec->data(Qt::UserRole).toULongLong() ? ((Class*) itemSelec->data(Qt::UserRole).toULongLong())->getId() : 0;
    else    m_saveIdSelecClass = 0;
}

void MainWindow::putSavedSelection() {
    for(int i=0; i<ui->list_kholleurs->count(); i++) {
        QListWidgetItem* item = ui->list_kholleurs->item(i);
        if(item) {
            Kholleur* khll = (Kholleur*) item->data(Qt::UserRole).toULongLong();
            if(khll && khll->getId() == m_saveIdSelecKholleur)
                ui->list_kholleurs->setCurrentRow(i);
        }
    }
    for(int i=0; i<ui->list_classes->count(); i++) {
        QListWidgetItem* item = ui->list_classes->item(i);
        if(item) {
            Class* cl = (Class*) item->data(Qt::UserRole).toULongLong();
            if(cl && cl->getId() == m_saveIdSelecClass)
                ui->list_classes->setCurrentRow(i);
        }
    }
}
