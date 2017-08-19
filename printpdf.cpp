#include "printpdf.h"

PrintPDF::PrintPDF()
{

}

bool PrintPDF::printTimeSlots(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db) {
    if(listClasses.length() <= 0)
        return false;

    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    "Kholles_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");
    if(filename == "")
        return false;

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setCreator("SPARK Secretariat");

    QPainter painter;
    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    for(int i=0; i<listClasses.length(); i++) {
        if(i != 0)
            qDebug() << writer.newPage();
        if(! drawPage(&writer, &painter, monday_date, listClasses[i], db))
            return false;
    }

    painter.end();
    QMessageBox::information(NULL, "Exportation terminée", "Le PDF a été généré.");
    return true;
}

bool PrintPDF::drawPage(QPdfWriter* writer, QPainter* painter, QDate monday_date, Class* cls, QSqlDatabase db) {
    //Get size in default units
    int width = writer->width();
    int height = writer->height();

    //Paint here
    painter->setPen(QPen(QBrush(Qt::black), 5));

    // Build the map associating an ID with its kholleur
    QMap<int, Kholleur*> kholleurs;
    QSqlQuery query(db);
    query.exec("SELECT id, name FROM sec_kholleurs");
    while (query.next()) {
        Kholleur* khll = new Kholleur();
        khll->setId(query.value(0).toInt());
        khll->setName(query.value(1).toString());
        kholleurs.insert(khll->getId(), khll);
    }

    QMap<QString, QMap<QDateTime, TimeSlot*>> timeslots;
    int nbRows = 0;

    query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle FROM sec_kholles WHERE "
                  "id_classes = :id_classes AND (date >= :start AND date <= :end) "
                  );
    query.bindValue(":id_classes", cls->getId());
    query.bindValue(":start", monday_date.toString("yyyy-MM-dd"));
    query.bindValue(":end", monday_date.addDays(6).toString("yyyy-MM-dd"));
    query.exec();

    while (query.next()) {
        TimeSlot* ts = new TimeSlot();
        ts->setId(query.value(0).toInt());
        ts->setDate(query.value(1).toDate());
        ts->setTime(query.value(2).toTime());
        ts->setNb_students(query.value(3).toInt());
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setId_classes(cls->getId());
        ts->setIs_exception(true);
        ts->setDuration_preparation(query.value(5).toInt());
        ts->setDuration_kholle(query.value(6).toInt());

        Kholleur* khll = kholleurs[ts->getId_kholleurs()];
        timeslots[khll->getName() + "_" + QString::number(khll->getId())][QDateTime(ts->getDate(), ts->getTime())] = ts;

        nbRows++;
    }

    query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle FROM sec_kholles WHERE "
                  "id_classes = :id_classes AND (date <= '1924-01-01') "
                  "AND id NOT IN "
                  "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                  "ORDER BY date, time"
                  );
    query.bindValue(":id_classes", cls->getId());
    query.bindValue(":monday", monday_date.toString("yyyy-MM-dd"));
    query.exec();

    while (query.next()) {
        TimeSlot* ts = new TimeSlot();
        ts->setId(query.value(0).toInt());
        QDate date = query.value(1).toDate();
        int numDays = QDate(1923, 1, 1).daysTo(date);
        ts->setDate(monday_date.addDays(numDays));
        ts->setTime(query.value(2).toTime());
        ts->setNb_students(query.value(3).toInt());
        ts->setId_kholleurs(query.value(4).toInt());
        ts->setId_classes(cls->getId());
        ts->setIs_exception(false);
        ts->setDuration_preparation(query.value(5).toInt());
        ts->setDuration_kholle(query.value(6).toInt());

        Kholleur* khll = kholleurs[ts->getId_kholleurs()];
        timeslots[khll->getName() + "_" + QString::number(khll->getId())][QDateTime(ts->getDate(), ts->getTime())] = ts;

        nbRows++;
    }

    qDebug() << nbRows;
    qDebug() << width;
    qDebug() << height;


    /// Data for the displaying
    // Introduction
    QString str_title = "Feuille de liaison";
    QString str_class = "Classe : " + cls->getName();
    QString str_week = "Semaine du Lundi " + monday_date.toString("dd/MM/yyyy");
    int heightTitle = width*2/30;
    int heightInfo = width/30;
    int heightIntro = heightTitle + heightInfo;

    // Footnotes
    QString str_footnote1 = "* Heure de préparation";
    QString str_footnote2 = "** Durée de passage en minutes. Entre parenthèse : durée de la préparation";
    int heightFootnotes = width*2/30;

    // Table
    QList<int> posLinesV;
    posLinesV << 0 << 33 << 55 << 75 << 90 << 100;
    QList<QString> titleColumns;
    titleColumns << "Kholleurs" << "Jour" << "Horaires*" << "Durée**" << "Elèves";
    int row_height = (height - heightIntro - heightFootnotes) / (nbRows+1);

    /// CALCUL POINT SIZE FONT
    // Point Size Font Title
    QFont titleFont = painter->font();
    titleFont.setBold(true);
    titleFont.setPointSize(adaptFont(&titleFont, str_title, width*4/5, heightTitle));

    // Point Size Font INFO
    QFont infoFont = painter->font();
    int ps_class = adaptFont(&infoFont, str_class, width*3/8, heightInfo);
    int ps_week = adaptFont(&infoFont, str_week, width*3/8, heightInfo);
    infoFont.setPointSize((ps_class>ps_week) ? ps_week : ps_class);

    // Point Size Font Headers
    QFont headersFont = painter->font();
    int min_psFont = -1;
    for(int i=0; i<titleColumns.length(); i++) {
        int ps = adaptFont(&headersFont, " " + titleColumns[i] + " ", (posLinesV[i+1]-posLinesV[i])*width/100, row_height);
        if(min_psFont < 0 || ps < min_psFont)
            min_psFont = ps;
    }
    headersFont.setPointSize(min_psFont);




    /// DRAWING...
    // Title
    painter->setFont(titleFont);
    QFontMetrics font = painter->fontMetrics();
    painter->drawText((width - font.width(str_title)) / 2, (heightTitle-font.height())/2 + font.ascent() + font.leading()/2, str_title);
    painter->setFont(infoFont);
    font = painter->fontMetrics();
    painter->drawText(0, heightTitle + (heightInfo-font.height())/2 + font.ascent() + font.leading()/2, str_class);
    painter->drawText(width/2, heightTitle + (heightInfo-font.height())/2 + font.ascent() + font.leading()/2, str_week);

    // Structure Table
    for(int i=0; i<posLinesV.length(); i++) {
        painter->drawLine(posLinesV[i]*width/100, heightIntro, posLinesV[i]*width/100, height-heightFootnotes);
    }
    for(int i=0; i<=nbRows+1; i++) {
        painter->drawLine(0, heightIntro+row_height*i, width, heightIntro+row_height*i);
    }
    painter->setFont(headersFont);
    QFontMetrics fontH = painter->fontMetrics();
    for(int i=0; i<titleColumns.length(); i++) {
        painter->drawText(posLinesV[i]*width/100, heightIntro + (row_height-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, " " + titleColumns[i] + " ");
    }

    // Display data
    QList<QMap<QDateTime, TimeSlot*>> tss = timeslots.values();
    QList<QList<TimeSlot*>> data;
    for(int i=0; i<tss.length(); i++) {
        data.append(tss[i].values());
    }

    int num = 0;
    for(int i=0; i<data.length(); i++) {
        for(int j=0; j<data[i].length(); j++) {
            TimeSlot* ts = data[i][j];
            int heightText = heightIntro + row_height + row_height*num + (row_height-fontH.height())/2 + fontH.ascent() + fontH.leading()/2;
            if(ts->getIs_exception())
                painter->setPen(QPen(QColor(Qt::red)));
            else
                painter->setPen(QPen(QColor(Qt::black)));

            painter->drawText(posLinesV[0]*width/100, heightText, " "+kholleurs[ts->getId_kholleurs()]->getName());
            painter->drawText(posLinesV[1]*width/100, heightText, " "+ts->getDate().toString("dddd dd/MM"));
            painter->drawText(posLinesV[2]*width/100, heightText, " "+ts->getTime().toString("hh:mm"));
            QString prep = (ts->getDuration_preparation()) ? " (" + QString::number(ts->getDuration_preparation()) + ")": "";
            painter->drawText(((posLinesV[3]+posLinesV[4])*width/100-font.width(QString::number(ts->getDuration_kholle()) + prep))/2, heightText, QString::number(ts->getDuration_kholle()) + prep);
            painter->drawText(((posLinesV[4]+posLinesV[5])*width/100-font.width(QString::number(ts->getNb_students())))/2, heightText, QString::number(ts->getNb_students()));

            num++;
        }
    }


    painter->drawText(0, height-heightFootnotes + (heightFootnotes/2-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, str_footnote1);
    painter->drawText(0, height-heightFootnotes + heightFootnotes/2 + (heightFootnotes/2-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, str_footnote2);


    // WILL FREE MEMORIES



    return true;
}

int PrintPDF::adaptFont(QFont* font, QString text, int widthText, int maxHeight) {
    int initialPointSize = font->pointSize();
    font->setPointSize(1);
    if(QFontMetrics(*font).lineSpacing() > maxHeight || QFontMetrics(*font).width(text) > widthText) {
        while((QFontMetrics(*font).lineSpacing() > maxHeight || QFontMetrics(*font).width(text) > widthText) && font->pointSize() > 1)
            font->setPointSize(font->pointSize() - 1);
    }
    if(QFontMetrics(*font).lineSpacing() < maxHeight && QFontMetrics(*font).width(text) < widthText) {
        while(QFontMetrics(*font).lineSpacing() <= maxHeight && QFontMetrics(*font).width(text) <= widthText)
            font->setPointSize(font->pointSize() + 1);
        font->setPointSize(font->pointSize() - 1);
    }
    int betterPointSize = font->pointSize();
    font->setPixelSize(initialPointSize);
    return betterPointSize;
}

bool PrintPDF::printKhollesPapers(QDate monday_date) {
    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    "Kholles_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");
    if(filename == "")
        return false;

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A3);
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setCreator("SPARK Secretariat");

    QPainter painter;
    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    int width = writer.width();
    int height = writer.height();

    /// Data for the displaying
    // Strings
    QStringList headers;
    headers << "NOM Prénom" << "Texte ou N° question" << "OBSERVATIONS";
    QString str_mark = "Note/20 :";
    QString str_obs = "OBSERVATIONS :";

    // Table Structure
    int nbRows = 9;
    int yTable = width*0.068;
    int heightText = width*0.03;
    int x2eColumn = width*0.18;
    int x3eColumn = width*0.42;
    QList<int> posLinesV;
    posLinesV << 0 << 18 << 42 << 100;

    int heightRow = (height-yTable-heightText) / (nbRows+1);
    int yEndTable = yTable + heightText + (nbRows+1)*heightRow;

    /// Style settings
    // Text Style
    QFont f = painter.font();
    f.setPointSize(16);
    f.setFamily("Times New Roman");
    f.setBold(true);
    painter.setFont(f);
    // Line Style
    painter.setPen(QPen(Qt::black, 30));
    // Tools
    QFontMetrics font = painter.fontMetrics();

    /// Headers of the paper...
    painter.drawText(0,0," Date :");
    painter.drawText(width*0.30,0,"Interrogateur :");
    painter.drawText(width*0.30,heightText*0.8,"Matière :");
    painter.drawText(width*0.30,heightText*0.8*2,"Classe :");
    painter.drawText(width*0.80,heightText*0.8,"Salle :");


    /// Table Structure
    // Headers
    painter.drawLine(0, yTable, width, yTable);
    painter.drawText((x2eColumn-font.width(headers[0]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[0]);
    painter.drawText((x2eColumn+x3eColumn-font.width(headers[1]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[1]);
    painter.drawText((x3eColumn+width-font.width(headers[2]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[2]);

    // H. Lines
    for(int i=0; i<=nbRows; i++) {
        int h = yTable + heightText + i*heightRow;
        painter.drawLine(0, h, width, h);
        if(i != 0) { // Mark area
            painter.drawText(0, h-heightText + (heightText-font.height())/2 + font.ascent() + font.leading()/2, " " + str_mark);
            painter.drawLine(0, h-heightText, x2eColumn, h-heightText);
        }
    }

    // Observation row
    painter.drawText((width-font.width(str_obs))/2, yEndTable-heightRow+(heightText-font.height())/2 + font.ascent() + font.leading()/2, str_obs);
    painter.drawLine(0, yEndTable, width, yEndTable);

    // V. Lines
    painter.drawLine(0, yTable, 0, yEndTable);
    painter.drawLine(width, yTable, width, yEndTable);
    painter.drawLine(x2eColumn, yTable, x2eColumn, yEndTable-heightRow);
    painter.drawLine(x3eColumn, yTable, x3eColumn, yEndTable-heightRow);


    return true;
}

