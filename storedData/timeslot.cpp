#include "timeslot.h"

TimeSlot::TimeSlot()
{
    m_id = 0;
    m_id_kholleurs = 0;
    m_id_classes = 0;
    m_date = QDate();
    m_time = QTime();
    m_nb_students = 0;
    m_is_exception = false;
    m_duration_preparation = 0;
    m_duration_kholle = 0;
    m_id_subjects = 0;
}

TimeSlot::~TimeSlot() {

}

//Getters
int TimeSlot::getId() const {
    return m_id;
}
int TimeSlot::getId_kholleurs() const {
    return m_id_kholleurs;
}
int TimeSlot::getId_classes() const {
    return m_id_classes;
}
QDate TimeSlot::getDate() const {
    return m_date;
}
QTime TimeSlot::getTime() const {
    return m_time;
}
int TimeSlot::getNb_students() const {
    return m_nb_students;
}
bool TimeSlot::getIs_exception() const {
    return m_is_exception;
}
int TimeSlot::getDuration_preparation() const {
    return m_duration_preparation;
}
int TimeSlot::getDuration_kholle() const {
    return m_duration_kholle;
}
int TimeSlot::getId_subjects() const {
    return m_id_subjects;
}

//Setters
void TimeSlot::setId(int id) {
    m_id = id;
}
void TimeSlot::setId_kholleurs(int id_kholleurs) {
    m_id_kholleurs = id_kholleurs;
}
void TimeSlot::setId_classes(int id_classes) {
    m_id_classes = id_classes;
}
void TimeSlot::setDate(QDate date) {
    m_date = date;
}
void TimeSlot::setTime(QTime time) {
    m_time = time;
}
void TimeSlot::setNb_students(int nb_students) {
    m_nb_students = nb_students;
}
void TimeSlot::setIs_exception(bool is_exception) {
    m_is_exception = is_exception;
}
void TimeSlot::setDuration_preparation(int duration_preparation) {
    m_duration_preparation = duration_preparation;
}
void TimeSlot::setDuration_kholle(int duration_kholle) {
    m_duration_kholle = duration_kholle;
}
void TimeSlot::setId_subjects(int id_subjects) {
    m_id_subjects = id_subjects;
}

