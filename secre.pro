QT += widgets sql concurrent network

FORMS += \
    mainwindow.ui \
    aboutitdialog.ui \
    contactdialog.ui \
    managers/kholleursmanager.ui \
    managers/classesmanager.ui \
    optionsdialog.ui \
    diffusionmanager.ui \
    printdialog.ui

HEADERS += \
    mainwindow.h \
    aboutitdialog.h \
    contactdialog.h \
    storedData/kholleur.h \
    storedData/class.h \
    managers/kholleursmanager.h \
    managers/classesmanager.h \
    weekbox.h \
    optionsdialog.h \
    onlinedatabase.h \
    diffusionmanager.h \
    printpdf.h \
    storedData/timeslot.h \
    printdialog.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    aboutitdialog.cpp \
    contactdialog.cpp \
    storedData/kholleur.cpp \
    storedData/class.cpp \
    managers/kholleursmanager.cpp \
    managers/classesmanager.cpp \
    weekbox.cpp \
    optionsdialog.cpp \
    onlinedatabase.cpp \
    diffusionmanager.cpp \
    printpdf.cpp \
    storedData/timeslot.cpp \
    printdialog.cpp

RESOURCES += \
    secre.qrc

DISTFILES +=
