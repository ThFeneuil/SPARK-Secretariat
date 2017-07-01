#include "storedData/kholleur.h"

Kholleur::Kholleur() {
    m_id = 0;
    m_name = "";
}

Kholleur::~Kholleur() {

}

//Getters
int Kholleur::getId() const {
    return m_id;
}
QString Kholleur::getName() const {
    return m_name;
}

// Setters
void Kholleur::setId(int id) {
    m_id = id;
}
void Kholleur::setName(QString name) {
    m_name = name;
}
