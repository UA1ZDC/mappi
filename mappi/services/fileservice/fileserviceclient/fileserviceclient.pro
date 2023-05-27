TEMPLATE = lib
TARGET = mappi.fileservice.client

HEADERS = fileserviceclient.h \
    filegetter.h

SOURCES = fileserviceclient.cpp \
    filegetter.cpp

LIBS =            -ltapp                        \
                  -ltdebug                      \
                  -lprotobuf                    \
                  -lmeteo.dbi                    \
                  -ltrpc                        \
                  -lmnprocread                  \
                  -lmeteo.global                \
                  -lmappi.global                \
                  -lmeteo.proto                 \
                  -lmappi.proto                 \
                  -lmappi.settings
                  
PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
