#ifndef WEEKBOX_H
#define WEEKBOX_H

#include <QObject>
#include <QGroupBox>
#include <QDate>
#include <QVboxLayout>
#include <QHboxLayout>
#include <QComboBox>
#include <QTimeEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QtSql>
#include <QDebug>
#include <QKeyEvent>
#include "storedData/kholleur.h"
#include "storedData/class.h"
#include "storedData/subject.h"
#include "utilities.h"

#define ALL_MONDAY QDate(1923,1,1)

class WeekBox;

class QListWidgetD : public QListWidget
{
    Q_OBJECT
public:
    explicit QListWidgetD(WeekBox* bx);

protected:
    bool eventFilter(QObject* obj, QEvent *event);

private:
    WeekBox* m_bx;
};

class WeekBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit WeekBox(QDate monday, Kholleur* khll, Class* cl, QSpinBox* spinbox_preparation, QSpinBox* spinbox_kholle, QComboBox* comboBox_subjects, QList<Subject*>* list_subjects, QWidget *parent = 0, QList<WeekBox*>* links = NULL);
    ~WeekBox();
    void displayHours();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

public slots:
    void updateSuffixNbStudents(int nb);
    void addHour();
    void deleteHour();
    void resetHours();

private:
    QDate m_monday;
    QComboBox* m_days;
    QTimeEdit* m_hour;
    QSpinBox* m_nbStudents;
    QListWidgetD* m_listHours;
    Kholleur* m_kholleur;
    Class* m_class;
    bool m_isWeekModel;
    QList<WeekBox*>* m_links;
    QSqlDatabase m_db;
    QSpinBox* m_spinbox_preparation;
    QSpinBox* m_spinbox_kholle;
    QComboBox* m_comboBox_subjects;
    QList<Subject*>* m_list_subjects;
    QMap<int, Subject*> m_subjects;
};

#endif // WEEKBOX_H
