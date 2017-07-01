#ifndef CLASS_H
#define CLASS_H


#include <QString>

class Class
{
public:
    Class();
    ~Class();

    //Getters
    int getId() const;
    QString getName() const;
    bool getOptServer() const;
    bool getOptEmail() const;
    bool getOptpaper() const;

    //Setters
    void setId(int id);
    void setName(QString name);
    void setOptServer(bool send);
    void setOptEmail(bool send);
    void setOptPaper(bool send);

private:
    int m_id;
    QString m_name;
    bool m_opt_server;
    bool m_opt_email;
    bool m_opt_paper;
};

#endif // CLASS_H
