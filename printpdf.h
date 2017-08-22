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

#define MAX_POINT_SIZE_TEXT_TIMESLOTS 17

class DiffusionManager;

class PrintPDF
{
public:
    PrintPDF();
    static bool drawPage(QPdfWriter *writer, QPainter *painter, QDate monday_date, Class *cls, QSqlDatabase db);
    static bool printTimeSlots(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db, DiffusionManager* manager = NULL);
    static bool printKhollesPapers(QDate monday_date);
    static void drawCenterText(QPainter *painter, int left, int right, int height, QString text);

private:
    static int adaptFont(QFont* font, QString text, int widthText, int maxHeight);

};

#endif // PRINTPDF_Hs
