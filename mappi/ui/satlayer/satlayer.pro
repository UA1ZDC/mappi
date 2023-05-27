TARGET   = satlayer
TEMPLATE = lib

QT       += widgets

HEADERS = satlayer.h \
    satlayermenu.h

SOURCES = satlayer.cpp \
    satlayermenu.cpp

LIBS =    -lmeteo.map

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

