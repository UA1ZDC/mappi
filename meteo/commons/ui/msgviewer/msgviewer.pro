TEMPLATE = lib
TARGET = meteo.msgviewer
QT += printsupport
QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS  += msgviewer.h \
    msgviewerdatamodel.h \
    lineedit.h  \
    leftpanel.h \
    pluginhandler.h

SOURCES +=  msgviewer.cpp \
    msgviewerdatamodel.cpp \
    lineedit.cpp \
    leftpanel.cpp \
    pluginhandler.cpp

FORMS    += leftpanel.ui

LIBS +=   -ltapp               \
          -ltdebug             \
          -lmeteo.nosql        \
          -lmeteo.global       \
          -lmeteo.proto        \
          -lmeteo.etc          \
          -lmeteo.viewheader   \
          -lmeteo.map.customui \
          -lmeteo.decodeviewer \
          -ltgribiface         \
          -lmeteo.customviewer \
          -lmeteo.map          \
          -lmeteo.map.view     \
          -lmeteo.map.dataexchange\
          -lmeteo.mainwindow  \
          -lmeteo.faxes \
          -lprimarydb

include( $$(SRC)/include.pro )

