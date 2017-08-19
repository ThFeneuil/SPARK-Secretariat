#include "printdialog.h"
#include "ui_printdialog.h"

PrintDialog::PrintDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton_print, SIGNAL(clicked(bool)), this, SLOT(print()));
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

void PrintDialog::print() {
    PrintPDF::printKhollesPapers(QDate(2017, 8, 21));
}
