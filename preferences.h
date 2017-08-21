#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

class Preferences {
public:
    Preferences();
    ~Preferences();

    bool initFile();
    bool read();
    bool write();

    // Setters
    bool setDir(QString dir);
    bool setFile(QString file);
    bool setServerDefault(bool isDefault);
    bool setServerScript(QString script);
    bool setServerPassword(QString password);

    // Getters
    QString dir();
    QString file();
    bool serverDefault();
    QString serverScript();
    QString serverPassword();

private:
    QString m_dir;
    QString m_file;
    bool m_serverDefault;
    QString m_serverScript;
    QString m_serverPassword;
};

#endif // PREFERENCES_H
