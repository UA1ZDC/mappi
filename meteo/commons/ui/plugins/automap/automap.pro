TEMPLATE = lib
TARGET = automap.plugin
QT += widgets printsupport
LOG_MODULE_NAME = vizdoc

SOURCES =       automaplugin.cpp


HEADERS =       automaplugin.h

LIBS +=         -lmeteo.etc              \
                -lmeteo.global           \
                -lmeteo.proto            \
                -lmeteo.settings         \
                -lmnfuncs                \
                -lprotobuf               \
                -lmeteo.planner          \
                -lmeteo.textproto        \
                -lmeteo.automap

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES +=



