TEMPLATE = app
TARGET   = mappi.file.service

HEADERS = fileservice.h

SOURCES = fileservice.cpp \
    main.cpp

LIBS =            -ltapp                        \
                  -ltdebug                      \
                  -lprotobuf                    \
                  -ltrpc                        \
                  -ltsingleton                  \
                  -lmnprocread                  \
                  -lmeteo.global                \
                  -lmeteo.settings              \
                  -lmeteo.dbi                   \
                  -lmappi.global                \
                  -lmeteo.proto                 \
                  -lmappi.proto                 \
                  -lmappi.settings

PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
