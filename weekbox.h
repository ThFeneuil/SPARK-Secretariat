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

class WeekBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit WeekBox(QDate monday, Kholleur* khll, Class* cl = NULL, QWidget *parent = 0);
    ~WeekBox();
    void displayHours();

protected:
    bool eventFilter(QObject* obj, QEvent *event);

public slots:
    void updateSuffixNbStudents(int nb);
    void addHour();
    void deleteHour();

private:
    QDate m_monday;
    QComboBox* m_days;
    QTimeEdit* m_hour;
    QSpinBox* m_nbStudents;
    QListWidget* m_listHours;
    Kholleur* m_kholleur;
    Class* m_class;
};

QString nameDay(int num);

#endif // WEEKBOX_H
