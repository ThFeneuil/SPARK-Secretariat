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
#include <QLabel>
#include "storedData/class.h"
#include "storedData/kholleur.h"
#include "storedData/timeslot.h"
#include "storedData/subject.h"
#include "diffusionmanager.h"
#include "utilities.h"

#define MAX_POINT_SIZE_TEXT_TIMESLOTS 12
#define MAX_STUDENTS_KP 9

class DiffusionManager;

class PrintPDF
{
public:
    PrintPDF();
    static bool drawPage(QPdfWriter *writer, QPainter *painter, QDate monday_date, Class *cls, QSqlDatabase db);
    static bool printTimeSlots(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db, DiffusionManager* manager = NULL);
    static void drawCenterText(QPainter *painter, int left, int right, int height, QString text);
    static QString limitedText(QPainter *painter, int widthMax, QString text);

    static bool initKhollesPapers(QDate monday_date, QList<Class *> listClasses, QSqlDatabase db, bool separateFiles);
    static bool printKhollesPapers(QString filename, QDate monday_date, QList<Class *> listClasses, QSqlDatabase db);
    static void drawKPStructure(QPdfWriter *writer, QPainter *painter);
    static void drawData(QPdfWriter *writer, QPainter *painter, QDate date, QString name_kh, QString name_sub, QString name_class, int nb_students);

private:
    static int adaptFont(QFont* font, QString text, int widthText, int maxHeight);

};

#endif // PRINTPDF_Hs
