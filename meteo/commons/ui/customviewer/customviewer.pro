TEMPLATE = lib
TARGET = meteo.customviewer
QT += printsupport
QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS  += contentwidget.h \
    addfilterdialog.h \        
    customviewerdatamodel.h \
    viewtablewidget.h

HEADERS += \
    customviewer.h \    
    customvieweritem.h

SOURCES += contentwidget.cpp \
    addfilterdialog.cpp \        
    customviewerdatamodel.cpp \
    viewtablewidget.cpp

SOURCES += \
    customviewer.cpp \
    customvieweritem.cpp

FORMS += \
    contentwidget.ui \
    addfilterdialog.ui

FORMS += \
    customviewer.ui

LIBS +=   -ltapp                \
          -ltdebug              \
          -lmeteo.nosql         \
          -lmeteo.global        \
          -lmeteo.proto         \
          -lmeteo.etc           \
          -lmeteo.viewheader    \
          -lmeteo.map.customui  \
          -lmeteo.decodeviewer  \
          -ltgribiface          \
          -lmeteo.ui.condition  \
          -lmeteo.prewiewwidget \
          -lmeteo.tlgeditor

include( $$(SRC)/include.pro )

