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

    // Getters
    QString dir();
    QString file();

private:
    QString m_dir;
    QString m_file;
};

#endif // PREFERENCES_H
