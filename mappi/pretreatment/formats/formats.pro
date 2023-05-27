TEMPLATE    =   lib
TARGET      =   mappi.po.formats

CONFIG += c++17

QMAKE_LFLAGS += -Wl,--no-as-needed

INCLUDEPATH += $$(SRC)/satdump/src-core

HEADERS     =   satformat.h \
                satwrapper.h \
                satpipeline.h

SOURCES     =   satformat.cpp \
                satwrapper.cpp \
                satpipeline.cpp

LIBS       +=   -lprotobuf \
                -ltdebug \
                -ltapp   \
                -lmeteo.textproto \
                -lmappi.proto \
                -lmeteo.geobasis \
                -lmnsatellite \
                -lmappi.global\
                -lmappi.po.images\
                -lsatdump_core


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

