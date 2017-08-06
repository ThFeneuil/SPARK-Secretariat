QT += widgets sql concurrent network

FORMS += \
    mainwindow.ui \
    aboutitdialog.ui \
    contactdialog.ui \
    managers/kholleursmanager.ui \
    managers/classesmanager.ui \
    optionsdialog.ui \
    diffusionmanager.ui

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
    diffusionmanager.h

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
    diffusionmanager.cpp

RESOURCES += \
    secre.qrc

DISTFILES +=
