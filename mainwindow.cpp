#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_list_kholleurs = new QList<Kholleur*>();
    m_list_classes = new QList<Class*>();
    ui->edit_kholleurs->installEventFilter(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data.db");
    if (!db.open()) {
        QMessageBox::critical(NULL, "Echec", "Impossible d'ouvrir la base de données...");
    }

    // Make the request
    QSqlQuery query(db);
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

    connect(ui->action_DB_Classes, SIGNAL(triggered()), this, SLOT(openClassesManager()));
    connect(ui->action_DB_Kholleurs, SIGNAL(triggered()), this, SLOT(openKholleursManager()));
    connect(ui->action_Help, SIGNAL(triggered()), this, SLOT(openHelp()));
    connect(ui->action_AboutIt, SIGNAL(triggered()), this, SLOT(openAboutIt()));
    connect(ui->edit_kholleurs, SIGNAL(textChanged(QString)), this, SLOT(selectKholleur(QString)));
    connect(ui->edit_classes, SIGNAL(textChanged(QString)), this, SLOT(selectClass(QString)));

    displayLists();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayLists() {
    for(int i=0; i<m_list_kholleurs->count(); i++) {
        Kholleur* khll = m_list_kholleurs->at(i);
        QListWidgetItem *item = new QListWidgetItem(khll->getName(), ui->list_kholleurs);
        item->setData(Qt::UserRole, (qulonglong) khll);
    }
    QListWidgetItem *itemAddKholleur = new QListWidgetItem("Ajouter ce kholleur", ui->list_kholleurs);
    itemAddKholleur->setFont(QFont("", -1, -1, QFont::italic));
    itemAddKholleur->setData(Qt::UserRole, (qulonglong) NULL);
    itemAddKholleur->setHidden(true);
    for(int i=0; i<m_list_classes->count(); i++) {
        Class* cl = m_list_classes->at(i);
        QListWidgetItem *item = new QListWidgetItem(cl->getName(), ui->list_classes);
        item->setData(Qt::UserRole, (qulonglong) cl);
    }
    QListWidgetItem *itemAddClass = new QListWidgetItem("Ajouter cette classe", ui->list_classes);
    itemAddClass->setFont(QFont("", -1, -1, QFont::italic));
    itemAddClass->setData(Qt::UserRole, (qulonglong) NULL);
    itemAddClass->setHidden(true);
}

void MainWindow::openClassesManager() {
    //Get connection information
    QSqlDatabase db = QSqlDatabase::database();

    if(db.isOpen()) {
        // Open the manager
        ClassesManager manager(&db, this);
        manager.exec();
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
            } else if(keyEvent->key() == Qt::Key_Enter) {

            }
        }
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}
