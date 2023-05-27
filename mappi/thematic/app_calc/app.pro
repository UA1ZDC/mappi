TARGET = mappi.thematic.app

CONFIG += console
CONFIG -= app_bundle
CONFIG += no_keywords #AsmJit use emit keyword

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
         -lmeteo.textproto          \
         -lmeteo.geobasis           \
         -lmeteo.dbi                \
         -lmappi.global             \
         -lmappi.proto              \
         -lmappi.settings           \
         -lasmjit            \
         -lmappi.them.algs_calc\
         -lmappi.landmask




PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include($$(SRC)/include.pro)

QMAKE_CXXFLAGS += -Wno-error=class-memaccess
