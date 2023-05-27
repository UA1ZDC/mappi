TEMPLATE = lib
TARGET   = mappi.histogrameditor
QT += widgets
QT += gui

DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

LOG_MODULE_NAME = histogrameditor

HEADERS +=     histogrameditor.h \
               histogramplugin.h \
               histogramaction.h


SOURCES +=     histogrameditor.cpp \
               histogramplugin.cpp \
               histogramaction.cpp

LIBS    +=  -lmappi.global        \
            -lmeteo.global        \
            -ltdebug              \
            -ltapp                \
            -lmeteo.map           \
            -lmeteo.map.view      \
            -ltrpc                \
            -lsatelliteimage      \
            -lsatlayer            \
            -lprotobuf            



FORMS += \
    histogrameditor.ui

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
