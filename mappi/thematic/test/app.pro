TARGET = test

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


HEADERS += 

SOURCES += main.cpp           


LIBS +=  -ltapp                     \
         -ltdebug                   \
         -lprotobuf                 \
         -ltrpc                     \
         -ltsingleton               \
         -lmnprocread               \
         -lmeteo.global             \
         -lmeteo.proto              \
         -lmeteo.settings           \
         -lmeteo.geobasis        \
         -lmeteo.etc             \
         -lmeteo.textproto          \
         -lmeteo.geobasis           \
         -lmeteo.dbi                \
         -lmappi.global             \
         -lmappi.proto              \
         -lmappi.settings           \
         -lmappi.landmask




PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include($$(SRC)/include.pro)

