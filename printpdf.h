#ifndef PRINTPDF_H
#define PRINTPDF_H

#include <QList>
#include <QMap>
#include <QFontMetrics>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPdfWriter>
#include <QPainter>
#include <QMessageBox>
#include <QtDebug>
#include <QCoreApplication>
#include <QtSql>
#include "storedData/class.h"
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"

class PrintPDF
{
public:
    PrintPDF();
    static bool drawPage(QPdfWriter *writer, QPainter *painter, QDate monday_date, Class *cls, QSqlDatabase db);
    static bool printTimeSlots(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db);
    //static void printKholles(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope);
    //static bool printKholles_StudentsDays(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename);
    //static bool printKholles_StudentsSubjects(QList<Student *> *students, QMap<int, Kholleur *> *kholleurs, QMap<int, Timeslot *> *timeslots, QDate monday_date, QMap<int, Kholle *> *kholloscope, QString filename);
    static int adaptFont(QFont* font, QString text, int widthText, int maxHeight);

private:
    //static int heightStudent(int numStudent, QList<QList<QList<Kholle*>*>*>* grid);
    //static bool writeTitle(QPainter* painter, int width, int maxHeight, QDate monday, double ratio = 4./5);
    //static double averageWidthStudents(QFontMetrics font, QList<Student *> *students);
    //static QString displayStudent(Student* s, int maxWidth, QFont font);
    //static void displaySPARK(QPainter* painter, int width, int height, int maxHeight, QFont normal);
};

#endif // PRINTPDF_Hs
