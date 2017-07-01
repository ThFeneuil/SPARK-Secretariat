#ifndef KHOLLEUR_H
#define KHOLLEUR_H

#include <QString>

class Kholleur
{
    public:
        Kholleur();
        ~Kholleur();

        //Getters
        int getId() const;
        QString getName() const;

        //Setters
        void setId(int id);
        void setName(QString name);

    private:
        int m_id;
        QString m_name;
};

#endif // KHOLLEUR_H
