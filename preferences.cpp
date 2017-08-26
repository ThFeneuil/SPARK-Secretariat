#include "preferences.h"

Preferences::Preferences() {
    m_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_serverDefault = true;
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
            } else if(data == "Default server:") {
                data = in.readLine();
                m_serverDefault = (data == "YES");
            } else if(data == "Server script:") {
                m_serverScript = in.readLine();
            } else if(data == "Server password:") {
                m_serverPassword = in.readLine();
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
        out << "Default server:\n" << (m_serverDefault ? "YES" : "NO") << "\n\n";
        out << "Server script:\n" << m_serverScript << "\n\n";
        out << "Server password:\n" << m_serverPassword << "\n\n";
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

bool Preferences::setServerDefault(bool isDefault) {
    read();
    m_serverDefault = isDefault;
    return write();
}

bool Preferences::setServerScript(QString script) {
    read();
    m_serverScript = script;
    return write();
}

bool Preferences::setServerPassword(QString password) {
    read();
    m_serverPassword = password;
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
bool Preferences::serverDefault() {
    read();
    return m_serverDefault;
}
QString Preferences::serverScript() {
    read();
    return m_serverScript;
}

QString Preferences::serverPassword() {
    read();
    return m_serverPassword;
}
