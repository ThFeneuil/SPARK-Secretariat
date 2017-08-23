#include "khollespapermanager.h"
#include "ui_khollespapermanager.h"

KhollesPaperManager::KhollesPaperManager(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KhollesPaperManager)
{
    ui->setupUi(this);
    m_db = db;
}

KhollesPaperManager::~KhollesPaperManager()
{
    delete ui;
}
