TEMPLATE = lib
TARGET   = mappi.brightness
DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

QT += widgets

#LOG_MODULE_NAME = brightness

HEADERS =     brightnessplugin.h \
    brightnessaction.h \
    brightnesscontrastwidget.h

SOURCES =     brightnessplugin.cpp \
    brightnessaction.cpp \
    brightnesscontrastwidget.cpp

LIBS    =    -ltdebug                    \
             -ltapp                      \
             -lmeteo.global              \
             -lmappi.global              \
             -lmeteo.map                 \
             -lmeteo.map.view            \
             -lsatlayer                  \
             -lsatelliteimage


LIBS +=      -L$(BUILD_DIR)/lib/plugins/mappi.app.widgets

DISTFILES = brightnessplugin.json

FORMS += \
    brightnesscontrastwidget.ui



PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
