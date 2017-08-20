#include "preferences.h"

Preferences::Preferences() {
    m_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    read();
}

Preferences::~Preferences() {

}

bool Preferences::read() {
    QFile read(QCoreApplication::applicationDirPath() + QDir::separator() + "preferences.pref");
    if(read.exists() && read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&read);
        while(! in.atEnd()) {
            QString data = in.readLine();
            if(data == "Dir path:") {
                m_dir = in.readLine();
                if(m_dir == "" || !QDir(m_dir).exists())
                    m_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            } else if(data == "File path:") {
                m_file = in.readLine();
                if(!QFile(m_file).exists())
                    m_file = "";
            }
        }
    }

    return true;
}

bool Preferences::write() {
    QFile write(QCoreApplication::applicationDirPath() + QDir::separator() + "preferences.pref");
    if(write.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream out(&write);
        out << "Dir path:\n" << m_dir << "\n\n";
        out << "File path:\n" << m_file << "\n\n";
        return true;
    }
    return false;
}

// Setters
bool Preferences::setDir(QString dir) {
    read();
    m_dir = dir;
    return write();
}

bool Preferences::setFile(QString file) {
    read();
    m_file = file;
    return write();
}

// Getters
QString Preferences::dir() {
    read();
    return m_dir;
}

QString Preferences::file() {
    read();
    return m_file;
}
