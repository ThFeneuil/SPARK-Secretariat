QT += widgets sql concurrent

FORMS += \
    mainwindow.ui \
    aboutitdialog.ui \
    contactdialog.ui \
    managers/kholleursmanager.ui \
    managers/classesmanager.ui

HEADERS += \
    mainwindow.h \
    aboutitdialog.h \
    contactdialog.h \
    storedData/kholleur.h \
    storedData/class.h \
    managers/kholleursmanager.h \
    managers/classesmanager.h \
    weekbox.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    aboutitdialog.cpp \
    contactdialog.cpp \
    storedData/kholleur.cpp \
    storedData/class.cpp \
    managers/kholleursmanager.cpp \
    managers/classesmanager.cpp \
    weekbox.cpp

RESOURCES +=
