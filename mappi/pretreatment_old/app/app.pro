TEMPLATE = app
TARGET   = mappi.pretreatment

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
         -lmappi.po.savenotify
         

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

