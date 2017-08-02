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
    return "";
}

WeekBox::WeekBox(QDate monday, Kholleur *khll, Class *cl, QWidget *parent, QList<WeekBox *> *links) : QGroupBox(parent)
{
    installEventFilter(this);

    m_monday = monday;
    m_kholleur = khll;
    m_class = cl;
    m_isWeekModel = (m_monday == ALL_MONDAY);
    m_links = links;
    m_db = QSqlDatabase::database();

    if(m_isWeekModel) {
        QPalette pal = palette();
        pal.setBrush(QPalette::Window, QBrush(QColor(0, 128, 0), Qt::Dense3Pattern));
        pal.setColor(QPalette::WindowText, QColor(255, 0, 0));
        pal.setColor(QPalette::AlternateBase, QColor(0, 0, 255));
        setAutoFillBackground(true);
        setPalette(pal);
    } else
        setTitle("Du " + m_monday.toString("dd/MM/yyyy") + " au " + m_monday.addDays(6).toString("dd/MM/yyyy"));

    m_days = new QComboBox;
    if(m_isWeekModel)
        for(int i=0; i<7; i++)
            m_days->addItem(m_monday.addDays(i).toString("dddd"), i);
    else
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
    if(! m_isWeekModel) {
        QPushButton* resetButton = new QPushButton("");
        QIcon icon(":/images/rollback.png");
        resetButton->setIcon(icon);
        connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(resetHours()));
        first_line->addWidget(resetButton);
        first_line->setStretch(0, 1);
        first_line->setStretch(1, 1);
        first_line->setStretch(2, 1);
        first_line->setStretch(3, 1);
        first_line->setStretch(4, 0);
    }

    m_listHours = new QListWidgetD(this);
    m_listHours->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

    QMap<QString, QListWidgetItem*> hours;

    if(m_class) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id, date, time, nb_students FROM sec_kholles WHERE "
                      "id_kholleurs = :id_kholleurs AND id_classes = :id_classes AND (date >= :start AND date <= :end) "
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
            QString text = "";
            if(m_isWeekModel)
                    text = nameDay(date.dayOfWeek()-1) + " " + time.toString("hh:mm") + " : " + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");
            else    text = nameDay(date.dayOfWeek()-1) + date.toString(" dd/MM/yyyy") + " " + time.toString("hh:mm") + " : " + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");

            QListWidgetItem* item = new QListWidgetItem(text);
            item->setData(Qt::UserRole, id);
            if(!m_isWeekModel)
                item->setForeground(QBrush(QColor(255,0,0)));

            hours[m_monday.toString("yyyy-MM-dd ") + time.toString("hh:mm ") + QString::number(id)] = item;
        }

        if(! m_isWeekModel) {
            query.prepare("SELECT id, date, time, nb_students FROM sec_kholles WHERE "
                          "id_kholleurs = :id_kholleurs AND id_classes = :id_classes AND (date <= '1924-01-01') "
                          "AND id NOT IN "
                          "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                          "ORDER BY date, time"
                          );
            query.bindValue(":id_kholleurs", m_kholleur->getId());
            query.bindValue(":id_classes", m_class->getId());
            query.bindValue(":monday", m_monday.toString("yyyy-MM-dd"));
            query.exec();

            while (query.next()) {
                int id = query.value(0).toInt();
                QDate date = query.value(1).toDate();
                int numDays = QDate(1923, 1, 1).daysTo(date);
                date = m_monday.addDays(numDays);
                QTime time = query.value(2).toTime();
                int nb_students = query.value(3).toInt();
                QString text = "";
                if(m_isWeekModel)
                        text = nameDay(date.dayOfWeek()-1) + " " + time.toString("hh:mm") + " : " + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");
                else    text = nameDay(date.dayOfWeek()-1) + date.toString(" dd/MM/yyyy") + " " + time.toString("hh:mm") + " : " + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");

                QListWidgetItem* item = new QListWidgetItem(text);
                item->setData(Qt::UserRole, -id);

                hours[date.toString("yyyy-MM-dd ") + time.toString("hh:mm ") + QString::number(id)] = item;
            }
        }

        QList<QListWidgetItem*> items = hours.values();
        for(int i=0; i<items.length(); i++)
            m_listHours->insertItem(m_listHours->count(), items[i]);
    }

    if(m_isWeekModel && m_links != NULL) {
        for(int i=1; i<m_links->length(); i++)
            if(m_links->at(i))
                m_links->at(i)->displayHours();
    }
}

void WeekBox::updateSuffixNbStudents(int nb) {
    if(nb >= 2)
            m_nbStudents->setSuffix(" élèves");
    else    m_nbStudents->setSuffix(" élève");
}

void WeekBox::addHour() {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO sec_kholles(id_kholleurs, id_classes, date, time, nb_students) VALUES(:id_kholleurs, :id_classes, :date, :time, :nb_students)");
    query.bindValue(":id_kholleurs", m_kholleur->getId());
    query.bindValue(":id_classes", m_class->getId());
    if(m_isWeekModel)
            query.bindValue(":date", m_monday.toString("yyyy-MM-dd"));
    else    query.bindValue(":date", m_monday.addDays(m_days->currentData(Qt::UserRole).toInt()).toString("yyyy-MM-dd"));
    query.bindValue(":time", m_hour->time().toString("hh:mm"));
    query.bindValue(":nb_students", m_nbStudents->value());
    query.exec();

    m_hour->setTime(m_hour->time().addSecs(3600));

    displayHours();
}

void WeekBox::deleteHour() {
    QList<QListWidgetItem*> selection = m_listHours->selectedItems();
    QStringList idsSpecial;
    QStringList idsAllMonday;
    for(int i=0; i<selection.length(); i++) {
        QListWidgetItem* item = selection[i];
        if(item) {
            int id = item->data(Qt::UserRole).toInt();
            if(id > 0) {
                idsSpecial << "id = " + QString::number(id);
            } else {
                id = -id;
                idsAllMonday << "(" + QString::number(id) + ", '" + m_monday.toString("yyyy-MM-dd") + "')";
            }
            m_listHours->takeItem(m_listHours->row(item));
            delete item;
        }
    }
    QSqlQuery query(m_db);
    if(idsSpecial.length() > 0)
        query.exec("DELETE FROM sec_kholles WHERE " + idsSpecial.join(" OR "));
    if(idsAllMonday.length() > 0)
        query.exec("INSERT INTO sec_exceptions(id_kholles, monday) VALUES" + idsAllMonday.join(", "));

    if(m_isWeekModel && m_links != NULL) {
        for(int i=1; i<m_links->length(); i++)
            if(m_links->at(i))
                m_links->at(i)->displayHours();
    }
}

void WeekBox::resetHours() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM sec_exceptions WHERE monday = :monday AND id_kholles IN "
                  "(SELECT id FROM sec_kholles WHERE id_kholleurs = :id_kholleurs AND id_classes = :id_classes)");
    query.bindValue(":monday", m_monday.toString("yyyy-MM-dd"));
    query.bindValue(":id_kholleurs", m_kholleur->getId());
    query.bindValue(":id_classes", m_class->getId());
    query.exec();

    query.prepare("DELETE FROM sec_kholles WHERE "
                  "id_kholleurs = :id_kholleurs AND id_classes = :id_classes AND (date >= :start AND date <= :end) ");
    query.bindValue(":id_kholleurs", m_kholleur->getId());
    query.bindValue(":id_classes", m_class->getId());
    query.bindValue(":start", m_monday.toString("yyyy-MM-dd"));
    query.bindValue(":end", m_monday.addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    displayHours();
}

bool WeekBox::eventFilter(QObject* obj, QEvent *event) {
    return QGroupBox::eventFilter(obj, event);
}

QListWidgetD::QListWidgetD(WeekBox* bx) : QListWidget() {
    installEventFilter(this);
    m_bx = bx;
}

bool QListWidgetD::eventFilter(QObject* obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            m_bx->deleteHour();
            return true;
        }
    }
    return QListWidget::eventFilter(obj, event);
}


