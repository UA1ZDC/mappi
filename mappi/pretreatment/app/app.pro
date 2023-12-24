TEMPLATE = app
TARGET   = mappi.pretreatment

CONFIG += c++17

INCLUDEPATH += $$(SRC)/satdump/src-core

SOURCES = main.cpp

LIBS +=  -lprotobuf \
         -ltapp   \
         -ltdebug \
         -lmnmathtools \
         -lmnsatellite \
         -lmeteo.geobasis  \
         -lmeteo.textproto \
         -lmappi.po.handler \
         -lmappi.po.formats \
         -lmappi.proto \
         -lmappi.global \
         -lmeteo.global \        
         -lmeteo.dbi \
         -lmappi.po.savenotify\
         -lmappi.po.images \
         -lmappi.them.algs_calc \
         -lsatdump_core
         

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

