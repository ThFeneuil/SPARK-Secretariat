QT += widgets sql concurrent network

FORMS += \
    mainwindow.ui \
    aboutitdialog.ui \
    contactdialog.ui \
    managers/kholleursmanager.ui \
    managers/classesmanager.ui \
    optionsdialog.ui \
    diffusionmanager.ui \
    printdialog.ui \
    managers/updatekholleurdialog.ui \
    managers/subjectsmanager.ui

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
    printdialog.h \
    managers/updatekholleurdialog.h \
    utilities.h \
    storedData/subject.h \
    managers/subjectsmanager.h

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
    printdialog.cpp \
    managers/updatekholleurdialog.cpp \
    utilities.cpp \
    storedData/subject.cpp \
    managers/subjectsmanager.cpp

RESOURCES += \
    secre.qrc

DISTFILES +=
