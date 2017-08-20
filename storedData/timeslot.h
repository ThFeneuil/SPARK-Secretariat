#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <QDateTime>

class TimeSlot
{
public:
    TimeSlot();
    ~TimeSlot();

    //Getters
    int getId() const;
    int getId_kholleurs() const;
    int getId_classes() const;
    QDate getDate() const;
    QTime getTime() const;
    int getNb_students() const;
    bool getIs_exception() const;
    int getDuration_preparation() const;
    int getDuration_kholle() const;
    int getId_subjects() const;

    //Setters
    void setId(int id);
    void setId_kholleurs(int id_kholleurs);
    void setId_classes(int id_classes);
    void setDate(QDate date);
    void setTime(QTime time);
    void setNb_students(int nb_students);
    void setIs_exception(bool is_exception);
    void setDuration_preparation(int duration_preparation);
    void setDuration_kholle(int duration_kholle);
    void setId_subjects(int id_subjects);

private:
    int m_id;
    int m_id_kholleurs;
    int m_id_classes;
    QDate m_date;
    QTime m_time;
    int m_nb_students;
    bool m_is_exception;
    int m_duration_preparation;
    int m_duration_kholle;
    int m_id_subjects;
};

#endif // TIMESLOT_H
