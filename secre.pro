QT += widgets sql concurrent network

FORMS += \
    mainwindow.ui \
    aboutitdialog.ui \
    managers/kholleursmanager.ui \
    managers/classesmanager.ui \
    optionsdialog.ui \
    diffusionmanager.ui \
    managers/updatekholleurdialog.ui \
    managers/subjectsmanager.ui \
    settingsdialog.ui \
    selectioninterface.ui \
    khollespapermanager.ui

HEADERS += \
    mainwindow.h \
    aboutitdialog.h \
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
    managers/updatekholleurdialog.h \
    utilities.h \
    storedData/subject.h \
    managers/subjectsmanager.h \
    storedData/parameterskholleurclass.h \
    preferences.h \
    settingsdialog.h \
    selectioninterface.h \
    khollespapermanager.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    aboutitdialog.cpp \
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
    managers/updatekholleurdialog.cpp \
    utilities.cpp \
    storedData/subject.cpp \
    managers/subjectsmanager.cpp \
    storedData/parameterskholleurclass.cpp \
    preferences.cpp \
    settingsdialog.cpp \
    selectioninterface.cpp \
    khollespapermanager.cpp

RESOURCES += \
    secre.qrc

DISTFILES +=

VERSION = 1.0
DEFINES += "APP_VERSION='$$VERSION'"
QMAKE_TARGET_PRODUCT = "SPARK Secretariat\0"

win32:RC_ICONS += "images/iconApp.ico"
