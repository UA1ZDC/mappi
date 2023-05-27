TEMPLATE = app
TARGET   = mappi.pretreatment.files

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
         -lmappi.po.images  \
         -lmappi.po.savenotify


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
         
include( $$(SRC)/include.pro )

