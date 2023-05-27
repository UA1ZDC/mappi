TEMPLATE = app
TARGET = mappi.schedule

SOURCES = main.cpp



LIBS += -lmappi.schedule \
        -lmappi.proto \
        -lmappi.global \
        -ltapp  \
        -ltdebug \
        -ltrpc

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
