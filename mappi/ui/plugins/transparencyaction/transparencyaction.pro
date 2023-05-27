TEMPLATE = lib
TARGET   = mappi.transparency
DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

QT += widgets

#LOG_MODULE_NAME = transparency

HEADERS = transparencyaction.h \
          transparencyplugin.h \
          transparencywidget.h

SOURCES = transparencyaction.cpp \
          transparencyplugin.cpp \
          transparencywidget.cpp

FORMS   = transparencywidget.ui

LIBS    =    -ltdebug                    \
             -ltapp                      \
             -lmeteo.global              \
             -lmappi.global              \
             -lmeteo.map                 \
             -lmeteo.map.view            \
             -lsatlayer                  \
             -lsatelliteimage

LIBS +=      -L$(BUILD_DIR)/lib/plugins/mappi.app.widgets

DISTFILES = transparencyplugin.json


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
