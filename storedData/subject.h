#ifndef SUBJECT_H
#define SUBJECT_H

#include <QString>

class Kholleur;
class Course;

class Subject
{
    public:
        Subject();
        ~Subject();

        //Getters
        int getId() const;
        QString getName() const;
        QString getShortName() const;

        //Setters
        void setId(int id);
        void setName(QString name);

    private:
        int m_id;
        QString m_name;
};

#endif // SUBJECT_H
