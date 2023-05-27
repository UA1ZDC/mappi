TEMPLATE = test

CONFIG += debug

SOURCES = main.cpp

LIBS +=  -lmappi.schedule \
         -lmeteo.global \
         -lmeteo.settings \
         -lmeteo.dbi \
         -lmappi.global \
         -ltapp
              
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )


