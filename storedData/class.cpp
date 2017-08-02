#include "class.h"

Class::Class() {
    m_id = 0;
    m_name = "";
    m_opt_server = false;
    m_opt_email = false;
    m_opt_paper = false;
    m_email = "";
}

Class::~Class() {

}

//Getters
int Class::getId() const {
    return m_id;
}
QString Class::getName() const {
    return m_name;
}
bool Class::getOptServer() const {
    return m_opt_server;
}
bool Class::getOptEmail() const {
    return m_opt_email;
}
bool Class::getOptPaper() const {
    return m_opt_paper;
}
QString Class::getEmail() const {
    return m_email;
}

// Setters
void Class::setId(int id) {
    m_id = id;
}
void Class::setName(QString name) {
    m_name = name;
}
void Class::setOptServer(bool send) {
    m_opt_server = send;
}
void Class::setOptEmail(bool send) {
    m_opt_email = send;
}
void Class::setOptPaper(bool send) {
    m_opt_paper = send;
}
void Class::setEmail(QString email) {
    m_email = email;
}
