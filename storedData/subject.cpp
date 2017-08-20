#include "storedData/subject.h"

Subject::Subject() {
    m_id = 0;
    m_name = "";
}

Subject::~Subject() {

}

//Getters
int Subject::getId() const {
    return m_id;
}
QString Subject::getName() const {
    return m_name;
}

//Setters
void Subject::setId(int id) {
    m_id = id;
}
void Subject::setName(QString name) {
    m_name = name;
}
