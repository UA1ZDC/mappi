TEMPLATE = app
TARGET   = mappi.pretreatment.files

CONFIG += c++17

INCLUDEPATH += $$(SRC)/satdump/src-core

SOURCES = main.cpp

LIBS +=  -lprotobuf \
         -ltapp     \
         -ltdebug   \
         -lmeteo.dbi   \
         -lmnmathtools \
         -lmnsatellite \
         -ltrpc             \ 
         -lmeteo.geobasis   \
         -lmeteo.textproto  \
         -lmeteo.global     \
         -lmeteo.settings   \
         -lmeteo.proto      \
         -lmappi.global     \
         -lmappi.settings   \
         -lmappi.po.handler \
         -lmappi.proto      \
         -lmappi.po.formats \
         -lmappi.po.savenotify \
         -lmappi.po.images \
         -lsatdump_core


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
         
include( $$(SRC)/include.pro )

