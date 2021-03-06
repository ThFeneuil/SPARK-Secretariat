#include "weekbox.h"

WeekBox::WeekBox(QDate monday, Kholleur *khll, Class *cl, QSpinBox* spinbox_preparation, QSpinBox* spinbox_kholle, QComboBox* comboBox_subjects, QList<Subject*>* list_subjects, QWidget *parent, QList<WeekBox *> *links) : QGroupBox(parent)
{
    installEventFilter(this);

    m_monday = monday;
    m_isWeekModel = (m_monday == ALL_MONDAY);
    m_links = links;
    m_db = QSqlDatabase::database();
    m_spinbox_preparation = spinbox_preparation;
    m_spinbox_kholle = spinbox_kholle;
    m_comboBox_subjects = comboBox_subjects;
    m_list_subjects = list_subjects;

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
            m_days->addItem(nameDay(i), i);
    else
        for(int i=0; i<7; i++)
            m_days->addItem(nameDay(m_monday.addDays(i).dayOfWeek()-1) + m_monday.addDays(i).toString(" dd/MM"), i);

    m_hour = new QTimeEdit(QTime(17, 0));
    m_hour->setDisplayFormat("HH:mm");
    m_hour->setToolTip("Heure de préparation");
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

    setKholleurClass(khll, cl);

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
    if(! m_class || ! m_kholleur)
        return;

    m_subjects.clear();
    for(int i=0; i<m_list_subjects->count(); i++)
        m_subjects[m_list_subjects->at(i)->getId()] = m_list_subjects->at(i);

    m_listHours->clear();

    QMap<QString, QListWidgetItem*> hours;

    if(m_class) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id, date, time, nb_students, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                      "id_kholleurs = :id_kholleurs AND id_classes = :id_classes AND (date >= :start AND date <= :end) "
                      "ORDER BY date, time"
                      );
        query.bindValue(":id_kholleurs", m_kholleur->getId());
        query.bindValue(":id_classes", m_class->getId());
        query.bindValue(":start", m_monday.toString("yyyy-MM-dd"));
        query.bindValue(":end", m_monday.addDays(6).toString("yyyy-MM-dd"));
        query.exec();

        while (query.next()) {
            QListWidgetItem* item = new QListWidgetItem();
            QFontMetrics font(item->font());

            int id = query.value(0).toInt();
            QDate date = query.value(1).toDate();
            QTime time = query.value(2).toTime();
            int nb_students = query.value(3).toInt();
            int duration_preparation = query.value(4).toInt();
            int duration_kholle = query.value(5).toInt();
            int id_subjects = query.value(6).toInt();
            QString str_subject = (id_subjects ? m_subjects[id_subjects]->getName() : "...");
            str_subject = str_subject.length() < 15 ? str_subject.leftJustified(15, ' ') : str_subject.left(11) + "...";
            while(font.width(str_subject) < 110)
                str_subject += " ";
            QString str_opt = str_subject+" \t"+QString::number(duration_preparation)+" min, "+QString::number(duration_kholle)+" min" + "       \t" + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");
            QString str_hour = "";
            if(m_isWeekModel)
                    str_hour = nameDay(date.dayOfWeek()-1) + " " + time.toString("hh:mm") + " (préparation)";
            else    str_hour = nameDay(date.dayOfWeek()-1) + date.toString(" dd/MM/yyyy") + " " + time.toString("hh:mm") + " (préparation)";

            QString text = str_hour + "\t       " + str_opt;

            item->setText(text);
            item->setData(Qt::UserRole, id);
            if(!m_isWeekModel)
                item->setForeground(QBrush(QColor(255,0,0)));

            hours[date.toString("yyyy-MM-dd ") + time.toString("hh:mm ") + QString::number(id)] = item;
        }

        if(! m_isWeekModel) {
            query.prepare("SELECT id, date, time, nb_students, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
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
                QListWidgetItem* item = new QListWidgetItem();
                QFontMetrics font(item->font());

                int id = query.value(0).toInt();
                QDate date = query.value(1).toDate();
                int numDays = QDate(1923, 1, 1).daysTo(date);
                date = m_monday.addDays(numDays);
                QTime time = query.value(2).toTime();
                int nb_students = query.value(3).toInt();
                int duration_preparation = query.value(4).toInt();
                int duration_kholle = query.value(5).toInt();
                int id_subjects = query.value(6).toInt();
                QString str_subject = (id_subjects ? m_subjects[id_subjects]->getName() : "...");
                str_subject = str_subject.length() < 15 ? str_subject.leftJustified(15, ' ') : str_subject.left(11) + "...";
                while(font.width(str_subject) < 110)
                    str_subject += " ";
                QString str_opt = str_subject+" \t"+QString::number(duration_preparation)+" min, "+QString::number(duration_kholle)+" min" + "       \t" + QString::number(nb_students) + ((nb_students >=2) ? " élèves" : " élève");
                QString str_hour = "";
                str_hour = nameDay(date.dayOfWeek()-1) + date.toString(" dd/MM/yyyy") + " " + time.toString("hh:mm") + " (préparation)";

                QString text = str_hour + "\t       " + str_opt;

                item->setText(text);
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
    if(! m_class || ! m_kholleur)
        return;

    Subject* subj = (Subject*) m_comboBox_subjects->currentData().toULongLong();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO sec_kholles(id_kholleurs, id_classes, date, time, nb_students, duration_preparation, duration_kholle, id_subjects) VALUES(:id_kholleurs, :id_classes, :date, :time, :nb_students, :duration_preparation, :duration_kholle, :id_subjects)");
    query.bindValue(":id_kholleurs", m_kholleur->getId());
    query.bindValue(":id_classes", m_class->getId());
    query.bindValue(":date", m_monday.addDays(m_days->currentData(Qt::UserRole).toInt()).toString("yyyy-MM-dd"));
    query.bindValue(":time", m_hour->time().toString("hh:mm"));
    query.bindValue(":nb_students", m_nbStudents->value());
    query.bindValue(":duration_preparation", m_spinbox_preparation->value());
    query.bindValue(":duration_kholle", m_spinbox_kholle->value());
    query.bindValue(":id_subjects", subj ? subj->getId() : 0);
    query.exec();

    m_hour->setTime(m_hour->time().addSecs(m_spinbox_kholle->value()*60));

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
    if(! m_class || ! m_kholleur)
        return;

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
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            addHour();
            return true;
        }
    }
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

void WeekBox::setKholleur(Kholleur* khll) {
    m_kholleur = khll;
    setVisible(m_kholleur != NULL && m_class != NULL);
    displayHours();
}

void WeekBox::setClass(Class* cl) {
    m_class = cl;
    setVisible(m_kholleur != NULL && m_class != NULL);
    displayHours();
}

void WeekBox::setKholleurClass(Kholleur* khll, Class* cl) {
    m_kholleur = khll;
    m_class = cl;
    setVisible(m_kholleur != NULL && m_class != NULL);
    displayHours();
}
