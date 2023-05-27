TEMPLATE = lib
TARGET   = mappi.mask
DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

QT += widgets

#LOG_MODULE_NAME = mask

FORMS = transparencyslider.ui

HEADERS = transparencyslider.h \
          maskaction.h \
          maskplugin.h

SOURCES = transparencyslider.cpp \
          maskaction.cpp \
          maskplugin.cpp

LIBS =    -ltdebug                    \
          -ltapp                      \
          -lmeteo.global              \
          -lmappi.global              \
          -lmeteo.map                 \
          -lmeteo.map.view


LIBS +=           -L$(BUILD_DIR)/lib/plugins/mappi.app.widgets

DISTFILES = maskplugin.json

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
