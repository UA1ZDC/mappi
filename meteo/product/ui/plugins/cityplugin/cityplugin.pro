TEMPLATE = lib
TARGET = city.plugin
QT += widgets
LOG_MODULE_NAME = vizdoc

SOURCES += cityplugin.cpp

HEADERS += cityplugin.h

LIBS +=                 -lmeteo.ui.city \
                        -lmeteo.wrb.map

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += cityplugin.json
