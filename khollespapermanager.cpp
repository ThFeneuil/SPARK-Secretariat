#include "khollespapermanager.h"
#include "ui_khollespapermanager.h"

KhollesPaperManager::KhollesPaperManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KhollesPaperManager)
{
    ui->setupUi(this);
    m_db = db;

    m_interface = new SelectionInterface(db, this);
    ui->layout->insertWidget(0, m_interface);

    connect(ui->btn_generate, SIGNAL(clicked(bool)), this, SLOT(printKhollesPaper()));
}

KhollesPaperManager::~KhollesPaperManager()
{
    delete ui;
    delete m_interface;
}

void KhollesPaperManager::printKhollesPaper() {
    bool separateFiles = ui->box_separateFiles->isChecked();
    QDate monday_date = m_interface->date();
    QList<QListWidgetItem*> selection = m_interface->classesSelectedItems();

    QList<Class*> listClasses;
    for(int i = 0; i < selection.length(); i++) {
        Class *c = (Class*) selection[i]->data(Qt::UserRole).toULongLong();
        listClasses.append(c);
    }

    PrintPDF::initKhollesPapers(monday_date, listClasses, *m_db, separateFiles);
}
