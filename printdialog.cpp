#include "printdialog.h"
#include "ui_printdialog.h"

PrintDialog::PrintDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);
    m_db = db;

    connect(ui->pushButton_print, SIGNAL(clicked(bool)), this, SLOT(print()));
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

void PrintDialog::print() {
    Class *c = new Class();
    c->setId(12);
    c->setName("MPSI2");
    c->setOptEmail(false);
    c->setOptPaper(true);
    c->setOptServer(false);
    QList<Class*> list;
    list << c;
    PrintPDF::printKhollesPapers(QDate(2017, 8, 28), list, *m_db);
}
