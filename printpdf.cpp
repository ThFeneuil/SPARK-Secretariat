#include "printpdf.h"

PrintPDF::PrintPDF()
{

}

bool PrintPDF::printTimeSlots(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db, DiffusionManager *manager) {
    if(listClasses.length() <= 0)
        return false;

    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    "HorairesKholles_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");
    if(filename == "") {
        if(manager)
            manager->writeDiffusionHistory("<strong>Création du PDF annulée...</strong>");
        return false;
    }

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A4);
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
    writer.setCreator("SPARK Secretariat");

    QPainter painter;
    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        if(manager)
            manager->writeDiffusionHistory("PDF ERROR : <br /><strong style='color:red'>Erreur lors de l'écriture du fichier.<br />Le fichier est peut-être ouvert ?</strong>");
        return false;
    }

    for(int i=0; i<listClasses.length(); i++) {
        if(i != 0)
            writer.newPage();
        if(! drawPage(&writer, &painter, monday_date, listClasses[i], db)) {
            if(manager)
                manager->writeDiffusionHistory("PDF ERROR : <br /><strong style='color:red'>Echec pour la page "+QString::number(i+1)+"/"+QString::number(listClasses.length())+"</strong>");
            return false;
        }
        if(manager)
            manager->writeDiffusionHistory("PDF : Page "+QString::number(i+1)+"/"+QString::number(listClasses.length())+" générée.");
    }

    painter.end();
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

    // Build the map associating an ID with its subjects
    QMap<int, Subject*> subjects;
    query.exec("SELECT id, name FROM sec_subjects");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subjects.insert(subj->getId(), subj);
    }
    Subject* emptySubject = new Subject();
    emptySubject->setId(0);
    emptySubject->setName("...");
    subjects.insert(0, emptySubject);

    QMap<QString, QMap<QString, QMap<QString, TimeSlot*>>> timeslots;
    int nbRows = 0;

    query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
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
        ts->setId_subjects(query.value(7).toInt());

        Kholleur* khll = kholleurs[ts->getId_kholleurs()];
        Subject* subj = subjects[ts->getId_subjects()];
        timeslots[khll->getName()+"_"+QString::number(khll->getId())][subj->getName()+"_"+QString::number(subj->getId())][QDateTime(ts->getDate(), ts->getTime()).toString("yyyy-MM-dd hh:mm")+QString::number(ts->getId())] = ts;

        nbRows++;
    }

    query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
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
        ts->setId_subjects(query.value(7).toInt());

        Kholleur* khll = kholleurs[ts->getId_kholleurs()];
        Subject* subj = subjects[ts->getId_subjects()];
        timeslots[khll->getName() + "_" + QString::number(khll->getId())][subj->getName()+"_"+QString::number(subj->getId())][QDateTime(ts->getDate(), ts->getTime()).toString("yyyy-MM-dd hh:mm")+QString::number(ts->getId())] = ts;

        nbRows++;
    }

    /// Data for the displaying
    // Introduction
    QString str_title = "Feuille de liaison";
    QString str_class = "Classe : " + cls->getName();
    QString str_week = "Semaine du Lundi " + monday_date.toString("dd/MM/yyyy");
    int heightTitle = width*2/30;
    int heightInfo = width/30;
    int heightSpace = width/60;
    int heightIntro = heightTitle + heightInfo+heightSpace;

    // Footnotes
    QString str_footnote1 = "* Heure de préparation";
    QString str_footnote2 = "** Durée de passage en minutes. Entre parenthèse : durée de la préparation";
    int heightFootnotes = width*2/30+heightSpace;

    // Table
    QList<int> posLinesV;
    posLinesV << 0 << 24 << 47 << 65 << 79 << 91 << 100;
    QList<QString> titleColumns;
    titleColumns << "Kholleurs" << "Matières" << "Jour" << "Horaires*" << "Durée**" << "Elèves";
    int row_height = (height - heightIntro - heightFootnotes) / (nbRows+1);
    int maxHeightRow = width*1/20;
    if(row_height > maxHeightRow)
        row_height = maxHeightRow;
    int yEndTable = row_height*(nbRows+1) + heightIntro;
    int thickLine = 18;
    int narrowLine = 7;

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

    // Point Size Font Text
    QFont headersFont = painter->font();
    headersFont.setPointSize(MAX_POINT_SIZE_TEXT_TIMESLOTS);
    headersFont.setBold(true);
    int min_psFont = -1;
    for(int i=0; i<titleColumns.length(); i++) {
        int ps = adaptFont(&headersFont, " " + titleColumns[i] + " ", (posLinesV[i+1]-posLinesV[i])*width/100, row_height);
        if(min_psFont < 0 || ps < min_psFont)
            min_psFont = ps;
    }
    headersFont.setPointSize(min_psFont);

    QFont textFont = headersFont;
    textFont.setBold(false);

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
        painter->setPen(QPen(QColor(Qt::black), (i==0 || i==posLinesV.length()-1) ? thickLine : narrowLine));
        painter->drawLine(posLinesV[i]*width/100, heightIntro, posLinesV[i]*width/100, yEndTable);
    }
    painter->setPen(QPen(QColor(Qt::black), thickLine));
    for(int i=0; i<2; i++)
        painter->drawLine(0, heightIntro+row_height*i, width, heightIntro+row_height*i);

    painter->setFont(headersFont);
    QFontMetrics fontH = painter->fontMetrics();
    int heightHeader = heightIntro + (row_height-fontH.height())/2 + fontH.ascent() + fontH.leading()/2;
    for(int i=0; i<titleColumns.length(); i++)
        drawCenterText(painter, posLinesV[i]*width/100, posLinesV[i+1]*width/100, heightHeader, titleColumns[i]);

    painter->setFont(textFont);

    // Display data
    QList<QList<QList<TimeSlot*>>> data;
    QMapIterator<QString, QMap<QString, QMap<QString, TimeSlot*>>> iTimeSlots(timeslots);
    while(iTimeSlots.hasNext()) {
        iTimeSlots.next();

        QMap<QString, QMap<QString, TimeSlot*>> map = iTimeSlots.value();
        QList<QMap<QString, TimeSlot*>> list = map.values();
        QList<QList<TimeSlot*>> doublelist;
        for(int j=0; j<list.length(); j++)
            doublelist.append(list[j].values());
        data.append(doublelist);
    }

    font = painter->fontMetrics();

    int num = 0;
    for(int i=0; i<data.length(); i++) {
        QString nameKholleur = "";
        int rowKholleurHeight = 0;
        for(int j=0; j<data[i].length(); j++) {
            QString nameSubject = "";
            int rowSubjectHeight = 0;
            for(int k=0; k<data[i][j].length(); k++) {
                TimeSlot* ts = data[i][j][k];
                int heightText = heightIntro + row_height + row_height*num + (row_height-fontH.height())/2 + fontH.ascent() + fontH.leading()/2;
                painter->setPen(ts->getIs_exception() ? QPen(QColor(Qt::red)) : QPen(QColor(Qt::black)));

                // Text
                nameKholleur = kholleurs[ts->getId_kholleurs()]->getName();
                nameSubject = subjects[ts->getId_subjects()]->getName();
                painter->drawText(posLinesV[2]*width/100, heightText, " "+nameDay(ts->getDate().dayOfWeek()-1)+ts->getDate().toString(" dd/MM"));
                drawCenterText(painter, posLinesV[3]*width/100, posLinesV[4]*width/100, heightText, ts->getTime().toString("hh:mm"));
                QString prep = (ts->getDuration_preparation()) ? " (" + QString::number(ts->getDuration_preparation()) + ")": "";
                drawCenterText(painter, posLinesV[4]*width/100, posLinesV[5]*width/100, heightText, QString::number(ts->getDuration_kholle()) + prep);
                drawCenterText(painter, posLinesV[5]*width/100, posLinesV[6]*width/100, heightText, QString::number(ts->getNb_students()));

                painter->setPen(QPen(QColor(Qt::black), narrowLine));
                num++;
                rowKholleurHeight += row_height;
                rowSubjectHeight += row_height;
                painter->drawLine(posLinesV[2]*width/100, heightIntro+row_height*(num+1), width, heightIntro+row_height*(num+1));
            }
            painter->drawText(posLinesV[1]*width/100, heightIntro + row_height + row_height*num-rowSubjectHeight + (rowSubjectHeight-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, limitedText(painter, (posLinesV[2]-posLinesV[1])*width/100, " "+nameSubject));
            painter->drawLine(posLinesV[1]*width/100, heightIntro+row_height*(num+1), posLinesV[2]*width/100, heightIntro+row_height*(num+1));
        }
        painter->drawText(posLinesV[0]*width/100, heightIntro + row_height + row_height*num-rowKholleurHeight + (rowKholleurHeight-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, limitedText(painter, (posLinesV[1]-posLinesV[0])*width/100, " "+nameKholleur));
        painter->drawLine(posLinesV[0]*width/100, heightIntro+row_height*(num+1), posLinesV[1]*width/100, heightIntro+row_height*(num+1));
        painter->setPen(QPen(QColor(Qt::black), thickLine));
        painter->drawLine(0, heightIntro+row_height*(num+1), width, heightIntro+row_height*(num+1));
    }

    textFont.setPointSize(MAX_POINT_SIZE_TEXT_TIMESLOTS);
    painter->setFont(textFont);
    painter->drawText(0, yEndTable +heightSpace+ (heightFootnotes/2-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, str_footnote1);
    painter->drawText(0, yEndTable +heightSpace+ (heightFootnotes-heightSpace)/2 + (heightFootnotes/2-fontH.height())/2 + fontH.ascent() + fontH.leading()/2, str_footnote2);

    // WILL FREE MEMORIES
    QMapIterator<int, Kholleur*> iKholleurs(kholleurs);
    while(iKholleurs.hasNext()) {
        iKholleurs.next();
        delete iKholleurs.value();
    }

    QMapIterator<int, Subject*> iSubjects(subjects);
    while(iSubjects.hasNext()) {
        iSubjects.next();
        delete iSubjects.value();
    }

    for(int i=0; i<data.length(); i++)
        for(int j=0; j<data[i].length(); j++)
            for(int k=0; k<data[i][j].length(); k++)
                delete data[i][j][k];

    return true;
}

QString PrintPDF::limitedText(QPainter *painter, int widthMax, QString text) {
    QFontMetrics font = painter->fontMetrics();
    QString newText = text + " ";
    if(font.width(newText) > widthMax)
        for(int i=text.length()-1; i>=0 && font.width(newText) > widthMax; i--)
            newText = text.left(i) + "... ";
    return newText;
}

void PrintPDF::drawCenterText(QPainter *painter, int left, int right, int height, QString text) {
    QFontMetrics font = painter->fontMetrics();
    painter->drawText((left+right-font.width(text))/2, height, text);
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
    font->setPointSize(initialPointSize);
    return betterPointSize;
}

bool PrintPDF::printKhollesPapers(QDate monday_date, QList<Class*> listClasses, QSqlDatabase db) {
    QString filename = QFileDialog::getSaveFileName(NULL, "Enregistrer sous...",
                                                    "Kholles_" + monday_date.toString("yyyyMMdd"),  "PDF (*.pdf)");
    if(filename == "")
        return false;

    //Create the PDF Writer
    QPdfWriter writer(filename);
    writer.setPageSize(QPdfWriter::A4);
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
    writer.setCreator("SPARK Secretariat");

    QPainter painter;
    if(!painter.begin(&writer)) {
        QMessageBox::critical(NULL, "Erreur", "Erreur lors de l'écriture du fichier. Le fichier est peut-être ouvert ?");
        return false;
    }

    /// Style settings
    // Text Style
    QFont f = painter.font();
    f.setFamily("Times New Roman");
    f.setBold(true);
    f.setPointSize(12);
    painter.setFont(f);
    // Line Style
    painter.setPen(QPen(Qt::black, 22));

    ///Get data
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

    // Build the map associating an ID with its subjects
    QMap<int, Subject*> subjects;
    query.exec("SELECT id, name FROM sec_subjects");
    while (query.next()) {
        Subject* subj = new Subject();
        subj->setId(query.value(0).toInt());
        subj->setName(query.value(1).toString());
        subjects.insert(subj->getId(), subj);
    }
    Subject* emptySub = new Subject();
    emptySub->setId(0);
    emptySub->setName("...");
    subjects.insert(emptySub->getId(), emptySub);

    bool first_page = true;
    for(int i = 0; i < listClasses.length(); i++) {
        QMap<int, QMap<int, QMap<QDate, int>>> pages;

        ///Load all data for this class
        query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                      "id_classes = :id_classes AND (date >= :start AND date <= :end) "
                      );
        query.bindValue(":id_classes", listClasses[i]->getId());
        query.bindValue(":start", monday_date.toString("yyyy-MM-dd"));
        query.bindValue(":end", monday_date.addDays(6).toString("yyyy-MM-dd"));
        query.exec();

        while(query.next()) {
            QDate date = query.value(1).toDate();
            int nb_students = query.value(3).toInt();
            int id_kholleurs = query.value(4).toInt();
            int id_subjects = query.value(7).toInt();

            if(!pages[id_kholleurs][id_subjects].contains(date))
                pages[id_kholleurs][id_subjects][date] = 0;

            pages[id_kholleurs][id_subjects][date] += nb_students;
        }

        query.prepare("SELECT id, date, time, nb_students, id_kholleurs, duration_preparation, duration_kholle, id_subjects FROM sec_kholles WHERE "
                      "id_classes = :id_classes AND (date <= '1924-01-01') "
                      "AND id NOT IN "
                      "(SELECT id_kholles FROM sec_exceptions WHERE monday=:monday) "
                      "ORDER BY date, time"
                      );
        query.bindValue(":id_classes", listClasses[i]->getId());
        query.bindValue(":monday", monday_date.toString("yyyy-MM-dd"));
        query.exec();

        while(query.next()) {
            QDate date = query.value(1).toDate();
            int numDays = QDate(1923, 1, 1).daysTo(date);
            date = monday_date.addDays(numDays);
            int nb_students = query.value(3).toInt();
            int id_kholleurs = query.value(4).toInt();
            int id_subjects = query.value(7).toInt();

            if(!pages[id_kholleurs][id_subjects].contains(date))
                pages[id_kholleurs][id_subjects][date] = 0;

            pages[id_kholleurs][id_subjects][date] += nb_students;
        }


        ///Treat data
        //For each kholleur
        QMapIterator<int, QMap<int, QMap<QDate, int>>> ikh(pages);
        while(ikh.hasNext()) {
            ikh.next();
            //For each subject
            QMapIterator<int, QMap<QDate, int>> isub(ikh.value());
            while(isub.hasNext()) {
                isub.next();
                //For each day
                QMapIterator<QDate, int> idate(isub.value());
                while(idate.hasNext()) {
                    idate.next();

                    if(!first_page)
                        writer.newPage();
                    first_page = false;

                    drawKPStructure(&writer, &painter);
                    drawData(&writer, &painter, idate.key(), kholleurs[ikh.key()], subjects[isub.key()], listClasses[i], idate.value());
                }
            }
        }
    }

    return true;
}

void PrintPDF::drawKPStructure(QPdfWriter *writer, QPainter *painter) {
    int width = writer->width();
    int height = writer->height();

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

    // Tools
    QFont f = painter->font();
    f.setBold(true);
    painter->setFont(f);
    QFontMetrics font = painter->fontMetrics();

    /// Headers of the paper...
    painter->drawText(0,0," Date :");
    painter->drawText(width*0.30,0,"Interrogateur :");
    painter->drawText(width*0.30,heightText*0.8,"Matière :");
    painter->drawText(width*0.30,heightText*0.8*2,"Classe :");
    painter->drawText(width*0.80,heightText*0.8,"Salle :");


    /// Table Structure
    // Headers
    painter->drawLine(0, yTable, width, yTable);
    painter->drawText((x2eColumn-font.width(headers[0]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[0]);
    painter->drawText((x2eColumn+x3eColumn-font.width(headers[1]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[1]);
    painter->drawText((x3eColumn+width-font.width(headers[2]))/2, yTable+(heightText-font.height())/2 + font.ascent() + font.leading()/2, headers[2]);

    // H. Lines
    for(int i=0; i<=nbRows; i++) {
        int h = yTable + heightText + i*heightRow;
        painter->drawLine(0, h, width, h);
        if(i != 0) { // Mark area
            painter->drawText(0, h-heightText + (heightText-font.height())/2 + font.ascent() + font.leading()/2, " " + str_mark);
            painter->drawLine(0, h-heightText, x2eColumn, h-heightText);
        }
    }

    // Observation row
    painter->drawText((width-font.width(str_obs))/2, yEndTable-heightRow+(heightText-font.height())/2 + font.ascent() + font.leading()/2, str_obs);
    painter->drawLine(0, yEndTable, width, yEndTable);

    // V. Lines
    painter->drawLine(0, yTable, 0, yEndTable);
    painter->drawLine(width, yTable, width, yEndTable);
    painter->drawLine(x2eColumn, yTable, x2eColumn, yEndTable-heightRow);
    painter->drawLine(x3eColumn, yTable, x3eColumn, yEndTable-heightRow);
}

void PrintPDF::drawData(QPdfWriter *writer, QPainter *painter, QDate date, Kholleur *kh, Subject *s, Class *c, int nb_students) {
    int width = writer->width();
    int heightText = width*0.03;

    QFontMetrics font = painter->fontMetrics(); //We need the fontmetrics for bold font, not normal font !!
    painter->drawText(font.width(" Date : "), heightText*0.8*2, " " + QString::number(nb_students) + (nb_students <= 1 ? " élève" : " élèves"));

    QFont f = painter->font();
    f.setBold(false);
    painter->setFont(f);

    painter->drawText(font.width(" Date : "), 0, nameDay(date.dayOfWeek()-1) + " " + date.toString("dd/MM/yyyy"));
    if(kh != NULL)
        painter->drawText(width*0.30 + font.width("Interrogateur : "), 0, kh->getName());
    if(s != NULL)
        painter->drawText(width*0.30 + font.width("Matière : "), heightText*0.8, s->getName());
    if(c != NULL)
        painter->drawText(width*0.30 + font.width("Classe : "), heightText*0.8*2, c->getName());
}
