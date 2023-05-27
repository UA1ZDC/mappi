TEMPLATE    = lib
TARGET      = mappi.po.savenotify

CONFIG += c++17

INCLUDEPATH += $$(SRC)/satdump/src-core

HEADERS     = savenotify.h


SOURCES     = savenotify.cpp


LIBS       +=   -ltdebug \
                -lmnmathtools \
                -lmnsatellite \
#               -lmeteo.proto \
                -lmappi.global     \
                -lmappi.settings   \
                -lmappi.proto \
                -lsatdump_core


PROTOPATH += $(SRC)/meteo/commons/proto              
include( $$(SRC)/protobuf.pri )          
include( $$(SRC)/include.pro )

