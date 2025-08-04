QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
    INCLUDEPATH += C:/eigen-3.4.0
}

unix {
    INCLUDEPATH += /usr/include/eigen3
}

SOURCES += \
    FormCStop.cpp \
    FormCTemp.cpp \
    FormCalib.cpp \
    FormPID.cpp \
    FormPrefs.cpp \
    View.cpp \
    devNano.cpp \
    main.cpp \
    MainWindow.cpp \
    probe.cpp

HEADERS += \
    FormCStop.h \
    FormCTemp.h \
    FormCalib.h \
    FormPID.h \
    FormPrefs.h \
    MainWindow.h \
    View.h \
    devNano.h \
    probe.h \
    state.h

FORMS += \
    FormCStop.ui \
    FormCTemp.ui \
    FormCalib.ui \
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

RESOURCES += \
    resources.qrc
