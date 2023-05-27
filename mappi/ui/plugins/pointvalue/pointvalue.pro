TEMPLATE        = lib
TARGET          = mappi.map.pointvalue
DESTDIR         = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

QT             += widgets

LOG_MODULE_NAME = pointvalue

HEADERS =         plugin.h                        \
                  pointvalueaction.h              \
                  balloonitem.h

SOURCES =         plugin.cpp                      \
                  pointvalueaction.cpp            \
                  balloonitem.cpp

OTHER_FILES =     pointvalueplugin.json

LIBS +=           -ltapp                          \
                  -ltdebug                        \

LIBS +=           -lmeteo.etc                     \
                  -lmeteo.global                  \
                  -lmeteo.proto                   \
                  -lmeteo.map                     \
                  -lmeteo.map.view                \
                  -lsatelliteimage

LIBS +=           -lsessionviewer.plugin          \

LIBS +=           -L$(BUILD_DIR)/lib/plugins/mappi.app.widgets


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
