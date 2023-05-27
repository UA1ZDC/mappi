TEMPLATE = lib
TARGET = mappi.msgviewer.plugin
LOG_MODULE_NAME = msgviewer
QT += printsupport
QT += widgets

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

HEADERS  += msgviewerplugin.h

SOURCES +=  msgviewerplugin.cpp

LIBS +=   -lmeteo.msgviewer \
          -lmeteo.mainwindow

include( $$(SRC)/include.pro )
