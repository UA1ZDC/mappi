TEMPLATE =  lib
TARGET =    bank.tlglog.plugin
QT +=       widgets
LOG_MODULE_NAME = applied

SOURCES = \
            tlglogplugin.cpp         \

HEADERS =  \
            tlglogplugin.h           \


LIBS =      -lmeteo.tlglog           \


include( $$(SRC)/include.pro )

DESTDIR =   $(BUILD_DIR)/lib/plugins/meteo.app.widgets


DISTFILES += \
    tlglog.json

