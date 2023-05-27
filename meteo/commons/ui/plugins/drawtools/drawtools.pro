TEMPLATE = lib
TARGET   = meteo.map.drawtools.plugin

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =               drawtoolsplugin.h             \

SOURCES =               drawtoolsplugin.cpp           \

LIBS +=                 -ltapp                        \

LIBS +=                 -lmeteo.etc                   \
                        -lmeteo.global                \
                        -lmeteo.map.view              \
                        -lmeteo.map.view.drawtools    \

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
