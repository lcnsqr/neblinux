QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/include/eigen3

SOURCES += \
    FormCStop.cpp \
    FormCTemp.cpp \
    FormCalib.cpp \
    FormFan.cpp \
    FormHeat.cpp \
    FormPID.cpp \
    FormPrefs.cpp \
    devNano.cpp \
    main.cpp \
    MainWindow.cpp \
    probe.cpp

HEADERS += \
    FormCStop.h \
    FormCTemp.h \
    FormCalib.h \
    FormFan.h \
    FormHeat.h \
    FormPID.h \
    FormPrefs.h \
    MainWindow.h \
    devNano.h \
    probe.h \
    state.h

FORMS += \
    FormCStop.ui \
    FormCTemp.ui \
    FormCalib.ui \
    FormFan.ui \
    FormHeat.ui \
    FormPID.ui \
    FormPrefs.ui \
    MainWindow.ui

TRANSLATIONS += \
    Util_pt_BR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES +=
