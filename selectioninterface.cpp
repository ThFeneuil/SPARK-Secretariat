#include "selectioninterface.h"
#include "ui_selectioninterface.h"

SelectionInterface::SelectionInterface(QSqlDatabase* db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectionInterface)
{
    ui->setupUi(this);
    m_db = db;

    QDate next_monday = QDate::currentDate();
    while(next_monday.dayOfWeek() != 1)
        next_monday = next_monday.addDays(1);
    ui->edit_monday->setDate(next_monday);

    QSqlQuery query(*m_db);

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

    connect(ui->list_classes, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));

    ui->list_classes->selectAll();
    update_list_mondays();
    connect(ui->edit_monday, SIGNAL(dateChanged(QDate)), this, SLOT(update_list_mondays()));
    connect(ui->list_mondays, SIGNAL(itemSelectionChanged()), this, SLOT(update_edit_monday()));
    connect(ui->list_mondays, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(boldSelection(QListWidgetItem*,QListWidgetItem*)));
    boldSelection(ui->list_mondays->currentItem(), NULL);
}

SelectionInterface::~SelectionInterface()
{
    delete ui;

    QMapIterator<int, Kholleur*> iKholleurs(m_kholleurs);
    while (iKholleurs.hasNext()) {
        iKholleurs.next();
        delete iKholleurs.value();
    }
    QMapIterator<int, Subject*> iSubjects(m_subjects);
    while (iSubjects.hasNext()) {
        iSubjects.next();
        delete iSubjects.value();
    }
    QMapIterator<int, Class*> iClasses(m_classes);
    while (iClasses.hasNext()) {
        iClasses.next();
        delete iClasses.value();
    }
}

void SelectionInterface::update_list_mondays() {
    QDate currentMonday = ui->edit_monday->date();

    if(currentMonday.dayOfWeek() != 1) {
        QMessageBox::information(this, "Correction de la date", "La date renseignée n'est pas un Lundi. Le lundi sélectionné sera donc le lundi de la même semaine que le jour rentré.");
        currentMonday = currentMonday.addDays(1-currentMonday.dayOfWeek());
    }

    ui->list_mondays->clear();
    QSqlQuery query(*m_db);
    for(int i=-10; i<13; i++) {
        QDate monday = currentMonday.addDays(7*i);
        QListWidgetItem* item = new QListWidgetItem(monday.toString("yyyy-MM-dd"), ui->list_mondays);
        item->setData(Qt::UserRole, monday);

        query.prepare("SELECT COUNT(*) FROM sec_backup_kholles WHERE date >= :start AND date <= :end");
        query.bindValue(":start", monday.toString("yyyy-MM-dd"));
        query.bindValue(":end", monday.addDays(6).toString("yyyy-MM-dd"));
        query.exec();
        if(query.next() &&  query.value(0).toInt() > 0)
            item->setBackgroundColor(QColor(255,128,0));
    }
    ui->list_mondays->setCurrentRow(10);
}

void SelectionInterface::update_edit_monday() {
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

void SelectionInterface::boldSelection(QListWidgetItem* current, QListWidgetItem* previous) {
    if(previous) {
        QFont fnt = previous->font();
        fnt.setBold(false);
        previous->setFont(fnt);
    }
    if(current){
        QFont fnt = current->font();
        fnt.setBold(true);
        current->setFont(fnt);
    }
}

void SelectionInterface::itemSelectionChanged() {
    emit classesSelectionChanged();
}

QList<QListWidgetItem*> SelectionInterface::classesSelectedItems() {
    return ui->list_classes->selectedItems();
}

QDate SelectionInterface::date() {
    return ui->edit_monday->date();
}
