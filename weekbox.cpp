#include "weekbox.h"

QString nameDay(int num) {
    switch(num) {
        case 0: return "Lundi";
        case 1: return "Mardi";
        case 2: return "Mercredi";
        case 3: return "Jeudi";
        case 4: return "Vendredi";
        case 5: return "Samedi";
        case 6: return "Dimanche";
    }
}

WeekBox::WeekBox(QDate monday, Kholleur *khll, Class *cl, QWidget *parent) : QGroupBox(parent)
{
    installEventFilter(this);

    m_monday = monday;
    m_kholleur = khll;
    m_class = cl;
    setTitle("Du " + m_monday.toString("dd/MM/yyyy") + " au " + m_monday.addDays(6).toString("dd/MM/yyyy"));
    setMinimumHeight(135);

    m_days = new QComboBox;
    for(int i=0; i<7; i++)
        m_days->addItem(m_monday.addDays(i).toString("dddd dd/MM"), i);
    m_hour = new QTimeEdit(QTime(17, 0));
    m_nbStudents = new QSpinBox;
    m_nbStudents->setMinimum(0);
    connect(m_nbStudents, SIGNAL(valueChanged(int)), this, SLOT(updateSuffixNbStudents(int)));
    m_nbStudents->setValue(3);
    QPushButton* addButton = new QPushButton("Ajouter");
    connect(addButton, SIGNAL(clicked(bool)), this, SLOT(addHour()));

    QHBoxLayout* first_line = new QHBoxLayout;
    first_line->addWidget(m_days);
    first_line->addWidget(m_hour);
    first_line->addWidget(m_nbStudents);
    first_line->addWidget(addButton);

    m_listHours = new QListWidget;
    displayHours();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(first_line);
    layout->addWidget(m_listHours);

    setLayout(layout);
}

WeekBox::~WeekBox() {
    delete m_days;
    delete m_hour;
    delete m_nbStudents;
}

void WeekBox::displayHours() {
    m_listHours->clear();

    if(m_class) {
        QSqlQuery query(QSqlDatabase::database());
        query.prepare("SELECT id, date, time, nb_students FROM sec_kholles WHERE "
                      "id_kholleurs = :id_kholleurs AND id_classes = :id_classes AND date >= :start AND date <= :end "
                      "ORDER BY date, time"
                      );
        query.bindValue(":id_kholleurs", m_kholleur->getId());
        query.bindValue(":id_classes", m_class->getId());
        query.bindValue(":start", m_monday.toString("yyyy-MM-dd"));
        query.bindValue(":end", m_monday.addDays(6).toString("yyyy-MM-dd"));
        query.exec();

        while (query.next()) {
            int id = query.value(0).toInt();
            QDate date = query.value(1).toDate();
            QTime time = query.value(2).toTime();
            int nb_students = query.value(3).toInt();
            QString text = nameDay(date.dayOfWeek()-1) + date.toString(" dd/MM/yyyy") + " " + time.toString("hh:mm") + " : " + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");
            QListWidgetItem* item = new QListWidgetItem(text, m_listHours);
            item->setData(Qt::UserRole, (qulonglong) id);
        }
    }
}

void WeekBox::updateSuffixNbStudents(int nb) {
    if(nb >= 2)
            m_nbStudents->setSuffix(" élèves");
    else    m_nbStudents->setSuffix(" élève");
}

void WeekBox::addHour() {
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("INSERT INTO sec_kholles(id_kholleurs, id_classes, date, time, nb_students) VALUES(:id_kholleurs, :id_classes, :date, :time, :nb_students)");
    query.bindValue(":id_kholleurs", m_kholleur->getId());
    query.bindValue(":id_classes", m_class->getId());
    query.bindValue(":date", m_monday.addDays(m_days->currentData(Qt::UserRole).toInt()).toString("yyyy-MM-dd"));
    query.bindValue(":time", m_hour->time().toString("hh:mm"));
    query.bindValue(":nb_students", m_nbStudents->value());
    query.exec();

    m_hour->setTime(m_hour->time().addSecs(3600));

    displayHours();
}

void WeekBox::deleteHour() {
    QListWidgetItem* item = m_listHours->currentItem();
    if(item) {
        int id = (int) item->data(Qt::UserRole).toULongLong();
        QSqlQuery query(QSqlDatabase::database());
        query.prepare("DELETE FROM sec_kholles WHERE id=:id");
        query.bindValue(":id", id);
        query.exec();
    }
}

bool WeekBox::eventFilter(QObject* obj, QEvent *event) {
    bool isAnItem = false;
    //for(int i=0; i<m_listHours->count(); i++)
    //    if(obj == m_listHours->item(i)->)
    //        isAnItem = true;

    if (isAnItem) {
        qDebug() << 4;

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete) {
                qDebug() << 3;
                deleteHour();
                return true;
            }
        }
        return false;
    }
    if(event->type() == QEvent::KeyPress)
        qDebug() <<42;
    return QGroupBox::eventFilter(obj, event);
}
