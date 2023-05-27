TEMPLATE = app
TARGET   = mappi.sessiondata.service

QT      += concurrent

HEADERS = sessiondataservice.h \
          methodbag.h

SOURCES = sessiondataservice.cpp \
          main.cpp \
          methodbag.cpp
          

LIBS =  -ltapp                 \
        -ltdebug               \
        -lprotobuf             \
        -ltrpc                 \
        -ltsingleton           \
        -lmnprocread           \
        -lmeteo.global         \
        -lmappi.global         \
        -lmeteo.proto          \
        -lmeteo.settings       \
        -lmeteo.dbi            \
        -lmappi.proto          \
        -lmappi.settings       \
        -lgdal

                  
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
