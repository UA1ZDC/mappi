QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

unix {
  INCLUDEPATH += /usr/include/qwt/
  LIBS =  -lqwt-qt5
  include(/usr/local/qwt-6.2.0/features/qwt.prf)
}

win32 {
  INCLUDEPATH += C:\Qwt-6.1.6\include
  include(C:\Qwt-6.1.6\features\qwt.prf);
}

CONFIG += qwt

CONFIG(release, debug|release) {
  message( "release" )
  LIBS += -lqwt
}
CONFIG(debug, debug|release) {
  message( "debug" )
  LIBS += -lqwtd
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    circularbuffer.cpp \
    drive.cpp \
    main.cpp \
    mainwindow.cpp \
    mytimer.cpp \
    plot.cpp \
    source/console.cpp \
    source/settingsdialog.cpp \
    vkaprotocol.cpp
    include/mytimer.cpp

HEADERS += \
    circularbuffer.h \
    drive.h \
    include/console.h \
    include/mytimer.h \
    include/settingsdialog.h \
    mainwindow.h \
    plot.h \
    protocol.h \
    vkaprotocol.h

FORMS += \
    mainwindow.ui \
    source/UI/settingsdialog.ui

greaterThan(QT_MAJOR_VERSION, 4) {

    QT += printsupport
    QT += concurrent
}

contains(QWT_CONFIG, QwtOpenGL ) {

    QT += opengl
}
else {

    DEFINES += QWT_NO_OPENGL
}

contains(QWT_CONFIG, QwtSvg) {

    QT += svg
}
else {

    DEFINES += QWT_NO_SVG
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    terminal.qrc \
    timer.qrc
