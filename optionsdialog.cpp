#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    m_db = db;
    m_canSave = true;

    connect(ui->list_classes, SIGNAL(currentRowChanged(int)), this, SLOT(selectClass()));
    connect(ui->checkbox_paper, SIGNAL(toggled(bool)), this, SLOT(optionsModified()));
    connect(ui->checkbox_email, SIGNAL(toggled(bool)), this, SLOT(optionsModified()));
    connect(ui->checkbox_server, SIGNAL(toggled(bool)), this, SLOT(optionsModified()));
    connect(ui->edit_email, SIGNAL(editingFinished()), this, SLOT(optionsModified()));

    update_list();
    selectClass();
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
    free_classes();
}

bool OptionsDialog::free_classes() {
    while (!queue_displayedClasses.isEmpty())
        delete queue_displayedClasses.dequeue();
    return true;
}

bool OptionsDialog::update_list() {
    // Clear the list
    ui->list_classes->clear();
    free_classes();

    // Make the request
    QSqlQuery query(*m_db);
    query.exec("SELECT id, name, by_server, by_email, by_paper, email FROM sec_classes ORDER BY UPPER(name)");

    // Treat the request
    while (query.next()) {
        Class* cl = new Class();
        cl->setId(query.value(0).toInt());
        cl->setName(query.value(1).toString());
        cl->setOptServer(query.value(2).toBool());
        cl->setOptEmail(query.value(3).toBool());
        cl->setOptPaper(query.value(4).toBool());
        cl->setEmail(query.value(5).toString());
        QListWidgetItem *item = new QListWidgetItem(cl->getName(), ui->list_classes);
        item->setData(Qt::UserRole, (qulonglong) cl);
        queue_displayedClasses.enqueue(cl);
    }

    return true;
}

bool OptionsDialog::selectClass() {
    QListWidgetItem* item = ui->list_classes->currentItem();

    m_canSave = false;
    if(item == NULL) {
        ui->label_name_class->setText("");
        ui->checkbox_paper->setEnabled(false);
        ui->checkbox_email->setEnabled(false);
        ui->checkbox_server->setEnabled(false);
    } else {
        Class* cls = (Class*) item->data(Qt::UserRole).toULongLong();

        ui->label_name_class->setText(cls->getName());
        ui->checkbox_paper->setChecked(cls->getOptPaper());
        ui->checkbox_email->setChecked(cls->getOptEmail());
        ui->checkbox_server->setChecked(cls->getOptServer());
        ui->edit_email->setText(cls->getEmail());

        ui->checkbox_paper->setEnabled(true);
        ui->checkbox_email->setEnabled(true);
        ui->checkbox_server->setEnabled(true);
        ui->edit_email->setEnabled(ui->checkbox_email->isChecked());
    }
    m_canSave = true;

    return true;
}

bool OptionsDialog::optionsModified() {
    ui->edit_email->setEnabled(ui->checkbox_email->isChecked());

    QListWidgetItem* item = ui->list_classes->currentItem();
    if(item && m_canSave) {
        Class* cls = (Class*) item->data(Qt::UserRole).toULongLong();

        cls->setOptPaper(ui->checkbox_paper->isChecked());
        cls->setOptEmail(ui->checkbox_email->isChecked());
        cls->setOptServer(ui->checkbox_server->isChecked());
        cls->setEmail(ui->edit_email->text());

        QSqlQuery query(*m_db);
        query.prepare("UPDATE sec_classes SET by_server=:by_server, by_email=:by_email, by_paper=:by_paper, email=:email WHERE id=:id");
        query.bindValue(":id", cls->getId());
        query.bindValue(":by_server", cls->getOptServer());
        query.bindValue(":by_email", cls->getOptEmail());
        query.bindValue(":by_paper", cls->getOptPaper());
        query.bindValue(":email", cls->getEmail());
        query.exec();
    }

    return true;
}
